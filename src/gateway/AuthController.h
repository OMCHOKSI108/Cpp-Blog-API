#pragma once

#include <drogon/HttpController.h>

namespace gateway {

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post);
        ADD_METHOD_TO(AuthController::signup, "/api/auth/signup", drogon::Post);
        ADD_METHOD_TO(AuthController::profile, "/api/auth/profile", drogon::Get);
    METHOD_LIST_END

    void login(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void signup(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void profile(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace gateway
