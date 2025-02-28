FROM ubuntu:24.04

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade

RUN apt-get install --no-install-recommends -y \
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
    xterm

COPY . /src
WORKDIR /build

# Build, test and install the main project
RUN cmake -S /src -B . -DBUILD_TESTS=ON \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr \
 && ctest --output-on-failure

# Build and install a plugin separately
RUN rm -rf * \
 && cmake /src/plugins/applications \
    -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr

ENTRYPOINT ["bash"]
