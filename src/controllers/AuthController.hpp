#pragma once
#include "../services/AuthService.hpp"
#include <drogon/HttpController.h>
#include <memory>

class AuthController : public drogon::HttpController<AuthController> {
  public:
    using HttpCallback = std::function<void(const drogon::HttpResponsePtr &)>;
    static void configure(std::shared_ptr<AuthService> service);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::registerUser, "/api/auth/register", drogon::Post);
    ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post);
    ADD_METHOD_TO(AuthController::googleUrl, "/api/auth/google/url", drogon::Get);
    ADD_METHOD_TO(AuthController::googleCallback, "/api/auth/google/callback", drogon::Post);
    METHOD_LIST_END

    void registerUser(const drogon::HttpRequestPtr &request,
                      HttpCallback &&callback) const;
    void login(const drogon::HttpRequestPtr &request,
               HttpCallback &&callback) const;
    void googleUrl(const drogon::HttpRequestPtr &request,
                   HttpCallback &&callback) const;
    void googleCallback(const drogon::HttpRequestPtr &request,
                        HttpCallback &&callback) const;

  private:
    static std::shared_ptr<AuthService> service_;
};
