#include "ReverseProxy.h"
#include <drogon/HttpClient.h>
#include <spdlog/spdlog.h>
#include "../analysis/TrafficAnalyzer.h"
#include "../ml/InferenceEngine.h"
#include "BanManager.h"

#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <iomanip>
#include <thread>
#include <openssl/sha.h>

namespace gateway {

static std::string g_backend_url = "http://httpbin.org";
static std::shared_mutex g_backend_mutex;
static GatewayConfig g_config;
static std::shared_mutex g_config_mutex;

static std::string computeClientId(const drogon::HttpRequestPtr& req) {
    std::string ip = req->peerAddr().toIp();
    std::string auth = req->getHeader("Authorization");
    std::string ua = req->getHeader("User-Agent");
    
    std::string combined = ip + "|" + auth + "|" + ua;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

void ReverseProxy::setGlobalBackendUrl(const std::string& url) {
    std::unique_lock lock(g_backend_mutex);
    g_backend_url = url;
    spdlog::info("Global Backend URL updated to: {}", url);
}

std::string ReverseProxy::getGlobalBackendUrl() {
    std::shared_lock lock(g_backend_mutex);
    return g_backend_url;
}

void ReverseProxy::setGlobalConfig(const GatewayConfig& config) {
    std::unique_lock lock(g_config_mutex);
    g_config = config;
    spdlog::info("Config: block={:.2f}, throttle={:.2f}, delay={}ms, ban={}s",
                 config.threshold_block, config.threshold_throttle,
                 config.throttle_max_delay_ms, config.ban_duration_seconds);
}

GatewayConfig ReverseProxy::getGlobalConfig() {
    std::shared_lock lock(g_config_mutex);
    return g_config;
}

// Shared logic for processing
void processRequest(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    const std::string& path_context) {
    
    // --- 1. Client Identification (Enhanced with hashing) ---
    std::string client_id = computeClientId(req);
    
    // --- 2. Check if banned ---
    if (BanManager::getInstance().isBanned(client_id)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k429TooManyRequests);
        resp->setBody("Too Many Requests: Temporarily Banned");
        callback(resp);
        return;
    }

    // --- 3. Traffic Analysis (with endpoint and payload tracking) ---
    size_t payload_size = req->body().length();
    auto metrics = analysis::TrafficAnalyzer::getInstance().updateAndGetMetrics(
        client_id, path_context, payload_size
    );
    
    // --- 4. AI Risk Scoring ---
    std::vector<float> features = {
        (float)metrics.rps,
        (float)metrics.burstiness,
        (float)metrics.endpoint_entropy,
        metrics.error_rate
    };
    float risk_score = ml::InferenceEngine::getInstance().predictRisk(features);
    
    // Update risk score in analyzer
    analysis::TrafficAnalyzer::getInstance().updateRiskScore(client_id, risk_score);

    spdlog::info("Client: {}, Path: {}, RPS: {:.2f}, Risk: {:.2f}, Entropy: {:.2f}", 
                 client_id, path_context, metrics.rps, risk_score, metrics.endpoint_entropy);

    // --- 5. Policy Enforcement (BLOCK / THROTTLE / ALLOW) ---
    GatewayConfig cfg = ReverseProxy::getGlobalConfig();
    
    if (risk_score > cfg.threshold_block) {
        // BLOCK and BAN
        BanManager::getInstance().addBan(client_id, cfg.ban_duration_seconds, risk_score);
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k403Forbidden);
        resp->setBody("Access Denied: High Risk Detected - Temporarily Banned");
        callback(resp);
        return;
    } else if (risk_score > cfg.threshold_throttle) {
        // THROTTLE - introduce delay proportional to risk
        int delay_ms = static_cast<int>((risk_score - cfg.threshold_throttle) / 
                                        (cfg.threshold_block - cfg.threshold_throttle) * 
                                        cfg.throttle_max_delay_ms);
        spdlog::warn("Throttling client: {} for {}ms (Risk: {:.2f})", client_id, delay_ms, risk_score);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        // Continue to forward after delay
    }

    // --- 5. Forwarding Logic ---
    std::string backend_url = ReverseProxy::getGlobalBackendUrl();
    
    // Init from ENV only once if needed, but for now we rely on default or setGlobalBackendUrl
    // Actually, we should check ENV once at startup in main, but lazy init here is fine too 
    // if we want to support env vars as initial value.
    // For simplicity: We trust getGlobalBackendUrl() which defaults to httpbin.org
    
    auto client = drogon::HttpClient::newHttpClient(backend_url);
    auto fwd_req = drogon::HttpRequest::newHttpRequest();
    fwd_req->setMethod(req->method());
    // Preserve original path or append? 
    // For a transparent proxy, we usually forward the same path.
    // If path_context is empty (root), we forward /.
    // If path_context is "foo", we forward /foo.
    // `req->path()` contains the full path! Use that.
    fwd_req->setPath(req->path());
    fwd_req->setBody(std::string(req->body()));
    
    for (const auto& header : req->getHeaders()) {
        fwd_req->addHeader(header.first, header.second);
    }

    client->sendRequest(fwd_req, 
        [callback, client_id](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            if (result == drogon::ReqResult::Ok) {
                // Track errors
                int status = resp->statusCode();
                if (status >= 400 && status < 500) {
                    analysis::TrafficAnalyzer::getInstance().recordError(client_id, true, false);
                } else if (status >= 500) {
                    analysis::TrafficAnalyzer::getInstance().recordError(client_id, false, true);
                }
                
                auto my_resp = drogon::HttpResponse::newHttpResponse();
                my_resp->setStatusCode(resp->statusCode());
                my_resp->setBody(std::string(resp->body()));
                // Copy headers, but skip connection/length headers that the framework handles
                for (auto& h : resp->getHeaders()) {
                    std::string key = h.first;
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    
                    if (key != "content-length" && 
                        key != "transfer-encoding" && 
                        key != "content-encoding" &&
                        key != "connection") {
                        my_resp->addHeader(h.first, h.second);
                    }
                }
                callback(my_resp);
            } else {
                auto err_resp = drogon::HttpResponse::newHttpResponse();
                err_resp->setStatusCode(drogon::k502BadGateway);
                err_resp->setBody("Bad Gateway");
                callback(err_resp);
            }
        });
}

void ReverseProxy::handlePath(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              std::string&& path) {
    processRequest(req, std::move(callback), path);
}

void ReverseProxy::handleRoot(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    processRequest(req, std::move(callback), "/");
}

} // namespace gateway
