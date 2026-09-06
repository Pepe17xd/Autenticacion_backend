#include "JwtManager.hpp"

#include <drogon/utils/Utilities.h>
#include <json/json.h>
#include <openssl/hmac.h>
#include <stdexcept>

namespace {
std::string encodeJson(const Json::Value &value) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return drogon::utils::base64Encode(Json::writeString(builder, value), true, false);
}
}

JwtManager::JwtManager(std::string secret, std::chrono::seconds lifetime)
    : secret_(std::move(secret)), lifetime_(lifetime) {
    if (secret_.size() < 32)
        throw std::invalid_argument("JWT_SECRET must contain at least 32 characters");
}

std::string JwtManager::createToken(const std::string &userId,
                                    const std::string &email) const {
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    const auto issuedAt = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    Json::Value payload;
    payload["sub"] = userId;
    payload["email"] = email;
    payload["iat"] = Json::Int64(issuedAt);
    payload["exp"] = Json::Int64(issuedAt + lifetime_.count());
    payload["iss"] = "astra-identity-service";
    const std::string unsignedToken = encodeJson(header) + "." + encodeJson(payload);
    unsigned int digestLength = 0;
    unsigned char digest[EVP_MAX_MD_SIZE];
    if (!HMAC(EVP_sha256(), secret_.data(), static_cast<int>(secret_.size()),
              reinterpret_cast<const unsigned char *>(unsignedToken.data()),
              unsignedToken.size(), digest, &digestLength))
        throw std::runtime_error("Unable to sign JWT");
    return unsignedToken + "." + drogon::utils::base64Encode(
        digest, digestLength, true, false);
}
