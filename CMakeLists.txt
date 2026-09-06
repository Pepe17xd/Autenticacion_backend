cmake_minimum_required(VERSION 3.20)

project(astra_identity_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-argon2 CONFIG QUIET)
if(NOT TARGET unofficial::argon2::libargon2)
    find_path(ARGON2_INCLUDE_DIR argon2.h REQUIRED)
    find_library(ARGON2_LIBRARY NAMES argon2 libargon2 REQUIRED)
    add_library(argon2_external UNKNOWN IMPORTED)
    set_target_properties(argon2_external PROPERTIES
        IMPORTED_LOCATION "${ARGON2_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ARGON2_INCLUDE_DIR}")
    add_library(unofficial::argon2::libargon2 ALIAS argon2_external)
endif()

add_executable(identity-service
    src/main.cpp
    src/controllers/AuthController.cpp
    src/services/AuthService.cpp
    src/repositories/UserRepository.cpp
    src/security/JwtManager.cpp
    src/security/PasswordHasher.cpp
    src/oauth/GoogleOAuth.cpp
)

target_include_directories(identity-service PRIVATE
    src
)

target_link_libraries(identity-service PRIVATE
    Drogon::Drogon
    OpenSSL::Crypto
    unofficial::argon2::libargon2
)
