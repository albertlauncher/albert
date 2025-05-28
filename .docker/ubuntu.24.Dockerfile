FROM ubuntu:24.04

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
    cmake \
    doctest-dev \
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
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-scxml-dev  \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-l10n-tools \
    qtkeychain-qt6-dev \
    xterm \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

COPY . /src

# Build, test and install the main project
RUN cmake -S /src -B /build -DBUILD_TESTS=ON \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

# Build and install a plugin separately
RUN cmake -S /src/plugins/applications -B /build -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

ENTRYPOINT ["bash"]
