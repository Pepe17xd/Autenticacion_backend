#pragma once

#include "../repositories/UserRepository.hpp"
#include "../security/JwtManager.hpp"
#include "../security/PasswordHasher.hpp"
#include "../oauth/GoogleOAuth.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string>

enum class AuthError { Validation, EmailExists, InvalidCredentials, Database, NotConfigured, NotImplemented };

struct AuthResult {
    std::optional<User> user;
    std::string accessToken;
    std::optional<AuthError> error;
    std::string message;
};

class AuthService {
  public:
    using Callback = std::function<void(AuthResult)>;
    AuthService(std::shared_ptr<UserRepository> repository,
                std::shared_ptr<PasswordHasher> passwordHasher,
                std::shared_ptr<JwtManager> jwtManager,
                std::shared_ptr<GoogleOAuth> googleOAuth);

    void registerUser(std::string name, std::string email,
                      std::string password, Callback callback) const;
    void login(std::string email, std::string password, Callback callback) const;
    std::optional<std::string> googleAuthorizationUrl() const;

  private:
    std::shared_ptr<UserRepository> repository_;
    std::shared_ptr<PasswordHasher> passwordHasher_;
    std::shared_ptr<JwtManager> jwtManager_;
    std::shared_ptr<GoogleOAuth> googleOAuth_;
};
