#include "AuthController.hpp"

#include <json/json.h>

std::shared_ptr<AuthService> AuthController::service_;

namespace {
drogon::HttpResponsePtr jsonResponse(Json::Value body,
                                     drogon::HttpStatusCode status) {
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(status);
    return response;
}

drogon::HttpResponsePtr errorResponse(const std::string &message,
                                      drogon::HttpStatusCode status) {
    Json::Value body;
    body["error"] = message;
    return jsonResponse(std::move(body), status);
}

drogon::HttpStatusCode statusFor(AuthError error) {
    switch (error) {
        case AuthError::Validation: return drogon::k400BadRequest;
        case AuthError::InvalidCredentials: return drogon::k401Unauthorized;
        case AuthError::EmailExists: return drogon::k409Conflict;
        case AuthError::NotConfigured: return drogon::k503ServiceUnavailable;
        case AuthError::NotImplemented: return drogon::k501NotImplemented;
        default: return drogon::k500InternalServerError;
    }
}

bool readString(const Json::Value &json, const char *field, std::string &destination) {
    if (!json.isMember(field) || !json[field].isString()) return false;
    destination = json[field].asString();
    return true;
}

Json::Value publicUser(const User &user) {
    Json::Value json;
    json["id"] = user.id;
    json["email"] = user.email;
    json["name"] = user.name;
    return json;
}
}

void AuthController::configure(std::shared_ptr<AuthService> service) {
    service_ = std::move(service);
}

void AuthController::registerUser(const drogon::HttpRequestPtr &request,
                                  HttpCallback &&callback) const {
    const auto json = request->getJsonObject();
    std::string name, email, password;
    if (!json || !readString(*json, "name", name) ||
        !readString(*json, "email", email) || !readString(*json, "password", password)) {
        callback(errorResponse("JSON fields name, email and password are required",
                               drogon::k400BadRequest));
        return;
    }
    service_->registerUser(std::move(name), std::move(email), std::move(password),
        [callback = std::move(callback)](AuthResult result) {
            if (result.error) return callback(errorResponse(result.message, statusFor(*result.error)));
            Json::Value body;
            body["user"] = publicUser(*result.user);
            callback(jsonResponse(std::move(body), drogon::k201Created));
        });
}

void AuthController::login(const drogon::HttpRequestPtr &request,
                           HttpCallback &&callback) const {
    const auto json = request->getJsonObject();
    std::string email, password;
    if (!json || !readString(*json, "email", email) || !readString(*json, "password", password)) {
        callback(errorResponse("JSON fields email and password are required",
                               drogon::k400BadRequest));
        return;
    }
    service_->login(std::move(email), std::move(password),
        [callback = std::move(callback)](AuthResult result) {
            if (result.error) return callback(errorResponse(result.message, statusFor(*result.error)));
            Json::Value body;
            body["accessToken"] = result.accessToken;
            body["user"] = publicUser(*result.user);
            callback(jsonResponse(std::move(body), drogon::k200OK));
        });
}

void AuthController::googleUrl(const drogon::HttpRequestPtr &,
                               HttpCallback &&callback) const {
    const auto url = service_->googleAuthorizationUrl();
    if (!url) return callback(errorResponse("Google OAuth is not configured",
                                            drogon::k503ServiceUnavailable));
    Json::Value body;
    body["url"] = *url;
    callback(jsonResponse(std::move(body), drogon::k200OK));
}

void AuthController::googleCallback(const drogon::HttpRequestPtr &,
                                    HttpCallback &&callback) const {
    callback(errorResponse("Google OAuth callback is prepared but not implemented",
                           drogon::k501NotImplemented));
}
