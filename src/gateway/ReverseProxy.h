#pragma once

#include <drogon/HttpController.h>

namespace gateway {

struct GatewayConfig {
    float threshold_block = 0.8f;
    float threshold_throttle = 0.5f;
    int throttle_max_delay_ms = 3000;
    int ban_duration_seconds = 300;
};

class ReverseProxy : public drogon::HttpController<ReverseProxy> {
public:
    METHOD_LIST_BEGIN
        // Catch-all path for everything else
        ADD_METHOD_TO(ReverseProxy::handlePath, "/{path}", drogon::Get, drogon::Post, drogon::Put, drogon::Delete, drogon::Options, drogon::Patch);
        // Handle root explicitely
        ADD_METHOD_TO(ReverseProxy::handleRoot, "/", drogon::Get, drogon::Post, drogon::Put, drogon::Delete, drogon::Options, drogon::Patch);
    METHOD_LIST_END

    void handlePath(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    std::string&& path);

    void handleRoot(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    static void setGlobalBackendUrl(const std::string& url);
    static std::string getGlobalBackendUrl();
    static void setGlobalConfig(const GatewayConfig& config);
    static GatewayConfig getGlobalConfig();
};

} // namespace gateway
