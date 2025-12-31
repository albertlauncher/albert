ARG BASE_IMAGE=ubuntu:24.04

FROM ${BASE_IMAGE} AS base
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    cmake \
    clang \
    clang-tools \
    ninja-build \
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
    qcoro-qt6-dev \
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
ARG build_dir="/build"
RUN cmake \
      -S /src \
      -B $build_dir \
      -G Ninja \
      -DBUILD_TESTS=ON \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
 && cmake --build $build_dir -j$(nproc) \
 && cmake --install $build_dir --prefix /usr \
 && ctest --test-dir $build_dir --output-on-failure \
 && rm -rf $build_dir

FROM build AS build-plugin
RUN cmake \
      -S /src/plugins/applications \
      -B $build_dir \
      -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build $build_dir -j$(nproc) \
 && cmake --install $build_dir --prefix /usr \
 && ctest --test-dir $build_dir --output-on-failure \
 && rm -rf $build_dir
