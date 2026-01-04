#pragma once

#include <drogon/HttpController.h>

namespace gateway {

class SettingsController : public drogon::HttpController<SettingsController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(SettingsController::updateBackend, "/api/config/backend", drogon::Post);
        ADD_METHOD_TO(SettingsController::getBackend, "/api/config/backend", drogon::Get);
    METHOD_LIST_END

    void updateBackend(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void getBackend(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace gateway
