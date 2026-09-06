#include "PasswordHasher.hpp"

#include <argon2.h>
#include <openssl/rand.h>
#include <array>
#include <stdexcept>
#include <vector>

namespace {
constexpr uint32_t kTimeCost = 3;
constexpr uint32_t kMemoryCostKiB = 65536;
constexpr uint32_t kParallelism = 1;
constexpr std::size_t kSaltLength = 16;
constexpr std::size_t kHashLength = 32;
}

std::string PasswordHasher::hash(const std::string &password) const {
    std::array<unsigned char, kSaltLength> salt{};
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
        throw std::runtime_error("Unable to generate a cryptographic salt");
    std::vector<char> encoded(argon2_encodedlen(kTimeCost, kMemoryCostKiB,
        kParallelism, kSaltLength, kHashLength, Argon2_id));
    const int result = argon2id_hash_encoded(kTimeCost, kMemoryCostKiB,
        kParallelism, password.data(), password.size(), salt.data(), salt.size(),
        kHashLength, encoded.data(), encoded.size());
    if (result != ARGON2_OK) throw std::runtime_error(argon2_error_message(result));
    return encoded.data();
}

bool PasswordHasher::verify(const std::string &password,
                            const std::string &encodedHash) const noexcept {
    return argon2id_verify(encodedHash.c_str(), password.data(), password.size()) == ARGON2_OK;
}
