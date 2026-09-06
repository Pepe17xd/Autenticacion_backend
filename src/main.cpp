#include "controllers/AuthController.hpp"
#include "oauth/GoogleOAuth.hpp"
#include "repositories/UserRepository.hpp"
#include "security/JwtManager.hpp"
#include "security/PasswordHasher.hpp"
#include "services/AuthService.hpp"

#include <drogon/drogon.h>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
constexpr std::string_view kFrontendOrigin = "http://localhost:5173";

std::string environment(const char *name, const std::string &fallback = {}) {
    const char *value = std::getenv(name);
    return value ? value : fallback;
}

void addSecurityHeaders(const drogon::HttpResponsePtr &response) {
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("X-Frame-Options", "DENY");
    response->addHeader("Cache-Control", "no-store");
}

void addCorsHeaders(const drogon::HttpRequestPtr &request,
                    const drogon::HttpResponsePtr &response) {
    // Only echo the explicitly trusted development origin; never use '*'
    // because authenticated API calls use the Authorization header.
    if (request->getHeader("Origin") == kFrontendOrigin) {
        response->addHeader("Access-Control-Allow-Origin", kFrontendOrigin.data());
        response->addHeader("Vary", "Origin");
    }
}
}

int main() {
    try {
        const auto databaseUrl = environment(
            "DATABASE_URL", "host=127.0.0.1 port=5434 dbname=identity_db user=identity password=identity");
        const auto jwtSecret = environment("JWT_SECRET");
        if (jwtSecret.empty())
            throw std::runtime_error("JWT_SECRET environment variable is required");

        auto db = drogon::orm::DbClient::newPgClient(databaseUrl, 4);
        auto repository = std::make_shared<UserRepository>(db);
        auto hasher = std::make_shared<PasswordHasher>();
        auto jwt = std::make_shared<JwtManager>(jwtSecret);
        auto google = std::make_shared<GoogleOAuth>(GoogleOAuthConfig{
            environment("GOOGLE_CLIENT_ID"), environment("GOOGLE_CLIENT_SECRET"),
            environment("GOOGLE_REDIRECT_URI")});
        AuthController::configure(std::make_shared<AuthService>(
            repository, hasher, jwt, google));

        // Handle preflight before routing so OPTIONS works for every current
        // and future API endpoint, even if it has no explicit OPTIONS route.
        drogon::app().registerPreRoutingAdvice(
            [](const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback,
               drogon::AdviceChainCallback &&next) {
                if (request->method() != drogon::Options) {
                    next();
                    return;
                }

                auto response = drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k204NoContent);
                addSecurityHeaders(response);
                addCorsHeaders(request, response);

                if (request->getHeader("Origin") == kFrontendOrigin) {
                    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
                    response->addHeader("Access-Control-Max-Age", "600");
                }
                callback(response);
            });

        drogon::app().registerPostHandlingAdvice(
            [](const drogon::HttpRequestPtr &request,
               const drogon::HttpResponsePtr &response) {
                addSecurityHeaders(response);
                addCorsHeaders(request, response);
            });
        drogon::app().addListener("0.0.0.0", 9000).setThreadNum(4).run();
        return 0;
    } catch (const std::exception &error) {
        LOG_ERROR << "Astra Identity Service failed to start: " << error.what();
        return 1;
    }
}
