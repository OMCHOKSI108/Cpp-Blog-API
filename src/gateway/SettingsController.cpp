#include "SettingsController.h"
#include "ReverseProxy.h"
#include <nlohmann/json.hpp>

namespace gateway {

void SettingsController::updateBackend(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = nlohmann::json::parse(req->bodyData(), nullptr, false);
    if (json.is_discarded()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string new_url = json.value("url", "");
    if (new_url.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("{\"error\":\"URL required\"}");
        callback(resp);
        return;
    }

    // Call static method on ReverseProxy to update global state
    ReverseProxy::setGlobalBackendUrl(new_url);

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody("{\"status\":\"updated\", \"url\":\"" + new_url + "\"}");
    callback(resp);
}

void SettingsController::getBackend(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string current_url = ReverseProxy::getGlobalBackendUrl();
    nlohmann::json j;
    j["url"] = current_url;
    
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(j.dump());
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(resp);
}

} // namespace gateway
