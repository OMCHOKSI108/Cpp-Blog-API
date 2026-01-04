#include "Dashboard.h"
#include "../analysis/TrafficAnalyzer.h"
#include "../auth/UserManager.h"
#include <nlohmann/json.hpp>

namespace gateway {

void Dashboard::getStats(const drogon::HttpRequestPtr& req, 
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    
    // Enforce authentication
    std::string token = req->getHeader("Authorization");
    if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }
    
    if (!auth::UserManager::getInstance().getUsernameFromToken(token)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody("{\"error\":\"Unauthorized\"}");
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }
    
    // Get stats from Analyzer
    auto all_metrics = analysis::TrafficAnalyzer::getInstance().getAllMetrics();

    // Convert to JSON
    nlohmann::json j;
    j["clients"] = nlohmann::json::array();
    
    // Global aggregates
    double total_rps = 0;
    int total_reqs = 0;
    float total_risk = 0.0f;
    int high_risk_count = 0;

    for (const auto& [client_id, metrics] : all_metrics) {
        j["clients"].push_back({
            {"client_id", client_id},
            {"rps", metrics.rps},
            {"burstiness", metrics.burstiness},
            {"total", metrics.total_requests},
            {"risk_score", metrics.risk_score},
            {"endpoint_entropy", metrics.endpoint_entropy},
            {"error_rate", metrics.error_rate},
            {"errors_4xx", metrics.error_count_4xx},
            {"errors_5xx", metrics.error_count_5xx}
        });
        total_rps += metrics.rps;
        total_reqs += metrics.total_requests;
        total_risk += metrics.risk_score;
        if (metrics.risk_score > 0.7f) high_risk_count++;
    }
    
    float avg_risk = all_metrics.empty() ? 0.0f : total_risk / all_metrics.size();

    j["global"] = {
        {"active_clients", all_metrics.size()},
        {"total_rps", total_rps},
        {"total_requests_tracked", total_reqs},
        {"avg_risk_score", avg_risk},
        {"high_risk_clients", high_risk_count}
    };

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(j.dump());
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(resp);
}

void Dashboard::getLogs(const drogon::HttpRequestPtr& req, 
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    // Enforce authentication
    std::string token = req->getHeader("Authorization");
    if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }
    
    if (!auth::UserManager::getInstance().getUsernameFromToken(token)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }
    
    nlohmann::json j = {
        {"logs", "Not implemented in this version (Requires log sink binding)"}
    };
    // In a real version, we'd bind spdlog to a ring buffer sink and read it here.
    
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(j.dump());
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(resp);
}

} // namespace gateway
