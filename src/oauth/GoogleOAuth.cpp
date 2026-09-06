#include "GoogleOAuth.hpp"

#include <drogon/utils/Utilities.h>

GoogleOAuth::GoogleOAuth(GoogleOAuthConfig config) : config_(std::move(config)) {}

bool GoogleOAuth::isConfigured() const noexcept {
    return !config_.clientId.empty() && !config_.clientSecret.empty() &&
           !config_.redirectUri.empty();
}

std::optional<std::string> GoogleOAuth::authorizationUrl() const {
    if (!isConfigured()) return std::nullopt;
    return "https://accounts.google.com/o/oauth2/v2/auth?client_id=" +
           drogon::utils::urlEncode(config_.clientId) + "&redirect_uri=" +
           drogon::utils::urlEncode(config_.redirectUri) +
           "&response_type=code&scope=openid%20email%20profile";
}
