#pragma once
#include <chrono>
#include <string>

class JwtManager {
  public:
    explicit JwtManager(std::string secret,
                        std::chrono::seconds lifetime = std::chrono::hours(1));
    std::string createToken(const std::string &userId,
                            const std::string &email) const;

  private:
    std::string secret_;
    std::chrono::seconds lifetime_;
};
