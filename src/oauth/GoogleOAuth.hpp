#pragma once
#include <optional>
#include <string>

struct GoogleOAuthConfig {
    std::string clientId;
    std::string clientSecret;
    std::string redirectUri;
};

class GoogleOAuth {
  public:
    explicit GoogleOAuth(GoogleOAuthConfig config);
    bool isConfigured() const noexcept;
    std::optional<std::string> authorizationUrl() const;

  private:
    GoogleOAuthConfig config_;
};
