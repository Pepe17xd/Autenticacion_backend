#pragma once
#include <string>

class PasswordHasher {
  public:
    std::string hash(const std::string &password) const;
    bool verify(const std::string &password,
                const std::string &encodedHash) const noexcept;
};
