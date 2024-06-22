FROM ubuntu:22.04 AS builder

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
        libmuparser-dev \
        libqalculate-dev \
        libqt6opengl6-dev \
        libqt6sql6-sqlite \
        libqt6svg6-dev \
        libxml2-utils \
        make \
        pkg-config \
        pybind11-dev \
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

# Build the entire project
RUN rm -rf * \
 && cmake /src -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_PLUGIN_MPRIS=OFF \
 && make -j $(nproc) \
 && make install

# Test build the apps plugin as separate project
RUN rm -rf * \
 && cmake /src/plugins/applications \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && make -j $(nproc) \
 && make install

ENTRYPOINT ["bash"]
