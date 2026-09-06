# syntax=docker/dockerfile:1.7

# =========================
# BUILD STAGE
# =========================
FROM ubuntu:24.04 AS build

ARG VCPKG_ROOT=/opt/vcpkg

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    bison \
    flex \
    ca-certificates \
    cmake \
    curl \
    git \
    ninja-build \
    pkg-config \
    python3 \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*


# Install vcpkg once. Dependency downloads and compiled packages are cached
# across BuildKit builds; a clean builder still produces the same image.
RUN --mount=type=cache,target=/root/.cache/vcpkg \
    git clone --depth 1 https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics


WORKDIR /app


# Copy the manifest first so application-only changes do not reinstall packages.
COPY vcpkg.json ./


# Instalar dependencias C++ desde vcpkg.json
RUN --mount=type=cache,target=/root/.cache/vcpkg \
    --mount=type=cache,target=/opt/vcpkg-bincache \
    VCPKG_BINARY_SOURCES="clear;files,/opt/vcpkg-bincache,readwrite" \
    ${VCPKG_ROOT}/vcpkg install \
    --triplet x64-linux \
    --x-manifest-root=/app


# Copy the application only after the dependency layer has been cached.
COPY CMakeLists.txt ./
COPY src ./src

# Configurar CMake
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_RPATH=/opt/vcpkg/installed/x64-linux/lib


# Compilar
RUN cmake --build build --parallel


# =========================
# RUNTIME STAGE
# =========================
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libpq5 \
    libssl3 \
    libargon2-1 \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app


# Binario compilado
COPY --from=build /app/build/identity-service /app/identity-service


# Librerías generadas por vcpkg
COPY --from=build /opt/vcpkg/installed/x64-linux/lib/ /usr/local/lib/


RUN ldconfig


EXPOSE 9000


CMD ["/app/identity-service"]
