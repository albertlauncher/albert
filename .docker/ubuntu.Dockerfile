ARG BASE_IMAGE=ubuntu:24.04

FROM ${BASE_IMAGE} AS base
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    ccache \
    cmake \
    g++ \
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
    qcoro-qt6-dev \
    qt6-base-dev \
    qt6-l10n-tools \
    qt6-scxml-dev  \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qtkeychain-qt6-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

FROM base AS dev
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    clangd \
    lldb \
    ninja-build \
    xterm \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

CMD ["bash"]
