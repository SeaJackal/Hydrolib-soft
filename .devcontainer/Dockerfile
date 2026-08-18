FROM mcr.microsoft.com/devcontainers/base:jammy

ARG CMAKE_VERSION=3.25.2
ARG CMAKE_ARCH=linux-x86_64
ARG LLVM_VERSION=19

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Install clangd from its APT repository instead of unpacking the complete LLVM
# distribution. Avoid upgrading the base image to keep builds small and stable.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        doxygen \
        gdb \
        lcov \
        libgtest-dev \
        python3-pip \
    && curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
        | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${LLVM_VERSION} main" \
        > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        clang-format-${LLVM_VERSION} \
        clang-tidy-${LLVM_VERSION} \
        clangd-${LLVM_VERSION} \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${LLVM_VERSION} 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-${LLVM_VERSION} 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${LLVM_VERSION} 100 \
    && python3 -m pip install --no-cache-dir compdb==0.2.0 \
    && curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_ARCH}.tar.gz" \
        | tar -xz --strip-components=1 -C /usr/local \
    && rm -rf /var/lib/apt/lists/*
