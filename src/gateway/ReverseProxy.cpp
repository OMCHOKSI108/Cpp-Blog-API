#include "ReverseProxy.h"
#include <drogon/HttpClient.h>
#include <spdlog/spdlog.h>
#include "../analysis/TrafficAnalyzer.h"
#include "../ml/InferenceEngine.h"

namespace gateway {

void ReverseProxy::handleRequest(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 std::string&& path) {
    
    // --- 1. Client Identification ---
    // In production, use a mix of IP + UserAgent + Authorization Header
    std::string client_ip = req->peerAddr().toIp();
    std::string user_agent = req->getHeader("User-Agent");
    std::string client_id = client_ip; // Simplified for demo

    // --- 2. Traffic Analysis ---
    auto metrics = analysis::TrafficAnalyzer::getInstance().updateAndGetMetrics(client_id);
    
    // --- 3. AI Risk Scoring ---
    std::vector<float> features = {
        (float)metrics.rps,
        (float)metrics.burstiness
    };
    float risk_score = ml::InferenceEngine::getInstance().predictRisk(features);

    spdlog::info("Client: {}, RPS: {:.2f}, Burst: {:.2f}, Risk: {:.2f}", 
                 client_id, metrics.rps, metrics.burstiness, risk_score);

    // --- 4. Policy Enforcement ---
    if (risk_score > 0.8) {
        // BLOCK
        spdlog::warn("Blocking abusive client: {}", client_id);
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k403Forbidden);
        resp->setBody("Access Denied: High Risk Detected");
        callback(resp);
        return;
    }

    if (risk_score > 0.5) {
        // THROTTLE (Mock implementation: Log it)
        spdlog::warn("Throttling suspicious client: {}", client_id);
        // Implementation note: In async C++, we would schedule the 'sendRequest' 
        // below to run after a delay (e.g., using a timer).
        // For this MVP, we just proceed but log it.
    }

    // --- 5. Forwarding Logic ---
    
    // Config: Load backend URL from global config or hardcoded for MVP
    // TODO: move to config loader
    static const std::string backend_url = "http://httpbin.org"; 

    auto client = drogon::HttpClient::newHttpClient(backend_url);
    auto fwd_req = drogon::HttpRequest::newHttpRequest();
    fwd_req->setMethod(req->method());
    fwd_req->setPath(req->path());
    fwd_req->setBody(req->body());
    
    for (const auto& header : req->getHeaders()) {
        fwd_req->addHeader(header.first, header.second);
    }

    client->sendRequest(fwd_req, 
        [callback](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            if (result == drogon::ReqResult::Ok) {
                // Return Backend Response via callback
                // We must create a new response to avoid safety issues if 'resp' is owned by client
                auto my_resp = drogon::HttpResponse::newHttpResponse();
                my_resp->setStatusCode(resp->statusCode());
                my_resp->setBody(std::string(resp->body())); // Copy body
                
                // Copy headers
                for (auto& h : resp->getHeaders()) {
                    my_resp->addHeader(h.first, h.second);
                }
                callback(my_resp);
            } else {
                spdlog::error("Backend request failed");
                auto err_resp = drogon::HttpResponse::newHttpResponse();
                err_resp->setStatusCode(drogon::k502BadGateway);
                err_resp->setBody("Bad Gateway");
                callback(err_resp);
            }
        });
}

} // namespace gateway
