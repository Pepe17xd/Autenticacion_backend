#include "AuthService.hpp"

#include <drogon/utils/Utilities.h>
#include <algorithm>
#include <cctype>
#include <regex>

namespace {
std::string trim(std::string value) {
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

bool validEmail(const std::string &email) {
    static const std::regex pattern(R"(^[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)+$)");
    return email.size() <= 254 && std::regex_match(email, pattern);
}

AuthResult failure(AuthError error, std::string message) {
    AuthResult result;
    result.error = error;
    result.message = std::move(message);
    return result;
}
}

AuthService::AuthService(std::shared_ptr<UserRepository> repository,
                         std::shared_ptr<PasswordHasher> passwordHasher,
                         std::shared_ptr<JwtManager> jwtManager,
                         std::shared_ptr<GoogleOAuth> googleOAuth)
    : repository_(std::move(repository)), passwordHasher_(std::move(passwordHasher)),
      jwtManager_(std::move(jwtManager)), googleOAuth_(std::move(googleOAuth)) {}

void AuthService::registerUser(std::string name, std::string email,
                               std::string password, Callback callback) const {
    name = trim(std::move(name));
    email = trim(std::move(email));
    std::transform(email.begin(), email.end(), email.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (name.empty() || name.size() > 150)
        return callback(failure(AuthError::Validation, "Name must contain between 1 and 150 characters"));
    if (!validEmail(email))
        return callback(failure(AuthError::Validation, "Email is invalid"));
    if (password.size() < 8 || password.size() > 128)
        return callback(failure(AuthError::Validation, "Password must contain between 8 and 128 characters"));

    auto completion = std::make_shared<Callback>(std::move(callback));
    repository_->findByEmail(email,
        [this, name = std::move(name), email = std::move(email),
         password = std::move(password), completion](std::optional<User> existing) mutable {
            if (existing) return (*completion)(failure(AuthError::EmailExists, "Email is already registered"));
            try {
                User user{drogon::utils::getUuid(), name, email,
                          passwordHasher_->hash(password), "local"};
                repository_->create(user,
                    [completion](User created) {
                        AuthResult result;
                        result.user = std::move(created);
                        (*completion)(std::move(result));
                    },
                    [completion](const std::string &) {
                        (*completion)(failure(AuthError::Database, "Unable to create user"));
                    });
            } catch (const std::exception &) {
                (*completion)(failure(AuthError::Database, "Unable to secure password"));
            }
        },
        [completion](const std::string &) {
            (*completion)(failure(AuthError::Database, "Unable to query users"));
        });
}

void AuthService::login(std::string email, std::string password, Callback callback) const {
    email = trim(std::move(email));
    if (!validEmail(email) || password.empty())
        return callback(failure(AuthError::Validation, "Email and password are required"));
    auto completion = std::make_shared<Callback>(std::move(callback));
    repository_->findByEmail(email,
        [this, password = std::move(password), completion](std::optional<User> user) mutable {
            if (!user || user->provider != "local" ||
                !passwordHasher_->verify(password, user->passwordHash))
                return (*completion)(failure(AuthError::InvalidCredentials, "Invalid email or password"));
            try {
                AuthResult result;
                result.accessToken = jwtManager_->createToken(user->id, user->email);
                result.user = std::move(user);
                (*completion)(std::move(result));
            } catch (const std::exception &) {
                (*completion)(failure(AuthError::Database, "Unable to create access token"));
            }
        },
        [completion](const std::string &) {
            (*completion)(failure(AuthError::Database, "Unable to query users"));
        });
}

std::optional<std::string> AuthService::googleAuthorizationUrl() const {
    return googleOAuth_->authorizationUrl();
}
