#pragma once
#include <drogon/orm/DbClient.h>
#include <functional>
#include <optional>
#include <string>

struct User {
    std::string id;
    std::string name;
    std::string email;
    std::string passwordHash;
    std::string provider;
};

class UserRepository {
  public:
    using ErrorCallback = std::function<void(const std::string &)>;
    explicit UserRepository(drogon::orm::DbClientPtr dbClient);
    void findByEmail(const std::string &email,
                     std::function<void(std::optional<User>)> onSuccess,
                     ErrorCallback onError) const;
    void create(const User &user, std::function<void(User)> onSuccess,
                ErrorCallback onError) const;

  private:
    drogon::orm::DbClientPtr dbClient_;
};
