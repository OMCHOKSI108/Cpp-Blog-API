#pragma once

#include <drogon/HttpController.h>

namespace gateway {

class ReverseProxy : public drogon::HttpController<ReverseProxy> {
public:
    METHOD_LIST_BEGIN
        // Catch-all path to handle every request
        ADD_METHOD_TO(ReverseProxy::handleRequest, "/{path}", drogon::Get, drogon::Post, drogon::Put, drogon::Delete, drogon::Options, drogon::Patch);
        // Also handle root
        ADD_METHOD_TO(ReverseProxy::handleRequest, "/", drogon::Get, drogon::Post, drogon::Put, drogon::Delete, drogon::Options, drogon::Patch);
    METHOD_LIST_END

    void handleRequest(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       std::string&& path);
};

} // namespace gateway
