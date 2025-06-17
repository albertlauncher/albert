ARG BASE_IMAGE=ubuntu:24.04

FROM ${BASE_IMAGE} AS base
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    cmake \
    clang \
    libarchive-dev \
    libgl1-mesa-dev \
    libglvnd-dev \
    libqalculate-dev \
    libqt6opengl6-dev \
    libqt6sql6-sqlite \
    libqt6svg6-dev \
    libxml2-utils \
    make \
    pkg-config \
    python3-dev \
    qt6-base-dev \
    qt6-scxml-dev  \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-l10n-tools \
    qtkeychain-qt6-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

FROM base AS dev
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    ccache \
    clangd \
    lldb \
    ninja-build \
    xterm \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

FROM base AS build
COPY . /src
ARG dir="/build"
RUN cmake -S /src -B $dir -DBUILD_TESTS=ON \
 && cmake --build $dir -j$(nproc) \
 && cmake --install $dir --prefix /usr \
 && ctest --test-dir $dir --output-on-failure \
 && rm -rf $dir

FROM base AS build-plugins
RUN cmake -S /src/plugins/applications -B $dir -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build $dir -j$(nproc) \
 && cmake --install $dir --prefix /usr \
 && ctest --test-dir $dir --output-on-failure \
 && rm -rf $dir
