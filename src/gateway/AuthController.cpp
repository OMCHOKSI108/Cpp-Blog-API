#include "AuthController.h"
#include "../auth/UserManager.h"
#include <nlohmann/json.hpp>

namespace gateway {

void AuthController::signup(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = nlohmann::json::parse(req->bodyData(), nullptr, false);
    if (json.is_discarded()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string user = json.value("username", "");
    std::string pass = json.value("password", "");
    std::string email = json.value("email", "");

    if (auth::UserManager::getInstance().registerUser(user, pass, email)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody("{\"status\":\"created\"}");
        callback(resp);
    } else {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k409Conflict);
        resp->setBody("{\"error\":\"User exists\"}");
        callback(resp);
    }
}

void AuthController::login(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = nlohmann::json::parse(req->bodyData(), nullptr, false);
    if (json.is_discarded()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string user = json.value("username", "");
    std::string pass = json.value("password", "");

    auto userObj = auth::UserManager::getInstance().login(user, pass);
    if (userObj) {
        std::string token = auth::UserManager::getInstance().createSession(user);
        nlohmann::json j;
        j["token"] = token;
        j["username"] = user;
        
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody(j.dump());
        callback(resp);
    } else {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody("{\"error\":\"Invalid credentials\"}");
        callback(resp);
    }
}

void AuthController::profile(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    // Check header
    std::string token = req->getHeader("Authorization");
    // Remove "Bearer " if present
    if (token.size() > 7 && token.substr(0,7) == "Bearer ") {
        token = token.substr(7);
    }

    auto usernameOpt = auth::UserManager::getInstance().getUsernameFromToken(token);
    if (!usernameOpt) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto userOpt = auth::UserManager::getInstance().getUser(*usernameOpt);
    if (userOpt) {
        nlohmann::json j;
        j["username"] = userOpt->username;
        j["email"] = userOpt->email;
        j["role"] = userOpt->role;
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody(j.dump());
        callback(resp);
    } else {
         auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
    }
}

} // namespace gateway
