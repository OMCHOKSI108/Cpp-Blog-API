#pragma once

#include <drogon/HttpController.h>

namespace gateway {

class Dashboard : public drogon::HttpController<Dashboard> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Dashboard::getStats, "/api/stats", drogon::Get);
        ADD_METHOD_TO(Dashboard::getLogs, "/api/logs", drogon::Get);
    METHOD_LIST_END

    void getStats(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void getLogs(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace gateway
