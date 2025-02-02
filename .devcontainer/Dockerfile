FROM mcr.microsoft.com/devcontainers/base:jammy

ARG USERNAME=hydrolib
ARG UID=1000
ARG GID=$UID

# Install some dependencies packages
RUN apt update -q \
    && apt upgrade -q -y \
    && apt install -y --no-install-recommends \
    build-essential \
    gdb \
    cmake \
    libgtest-dev \
    && apt clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
