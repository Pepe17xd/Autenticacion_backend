#include "UserRepository.hpp"

#include <exception>

namespace {
User mapUser(const drogon::orm::Row &row) {
    return {row["id"].as<std::string>(), row["name"].as<std::string>(),
            row["email"].as<std::string>(),
            row["password_hash"].as<std::string>(),
            row["provider"].as<std::string>()};
}

std::string exceptionMessage(const std::exception_ptr &error) {
    try { std::rethrow_exception(error); }
    catch (const std::exception &exception) { return exception.what(); }
    catch (...) { return "Unknown database error"; }
}
}

UserRepository::UserRepository(drogon::orm::DbClientPtr dbClient)
    : dbClient_(std::move(dbClient)) {}

void UserRepository::findByEmail(const std::string &email,
                                 std::function<void(std::optional<User>)> onSuccess,
                                 ErrorCallback onError) const {
    dbClient_->execSqlAsync(
        "SELECT id, name, email, password_hash, provider FROM users "
        "WHERE lower(email) = lower($1) LIMIT 1",
        [onSuccess = std::move(onSuccess)](const drogon::orm::Result &result) {
            onSuccess(result.empty() ? std::optional<User>{}
                                     : std::optional<User>{mapUser(result.front())});
        },
        [onError = std::move(onError)](const std::exception_ptr &error) {
            onError(exceptionMessage(error));
        }, email);
}

void UserRepository::create(const User &user, std::function<void(User)> onSuccess,
                            ErrorCallback onError) const {
    dbClient_->execSqlAsync(
        "INSERT INTO users (id, name, email, password_hash, provider) "
        "VALUES ($1::uuid, $2, lower($3), $4, $5) "
        "RETURNING id, name, email, password_hash, provider",
        [onSuccess = std::move(onSuccess)](const drogon::orm::Result &result) {
            onSuccess(mapUser(result.front()));
        },
        [onError = std::move(onError)](const std::exception_ptr &error) {
            onError(exceptionMessage(error));
        }, user.id, user.name, user.email, user.passwordHash, user.provider);
}
