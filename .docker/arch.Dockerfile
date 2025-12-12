ARG BASE_IMAGE=archlinux:latest

FROM ${BASE_IMAGE} AS base
RUN pacman -Syu --verbose --noconfirm \
    cmake \
    clang \
    ninja \
    libarchive \
    libqalculate \
    libxml2 \
    python \
    pkgconf \
    qt6-base \
    qt6-declarative \
    qt6-scxml \
    qt6-svg \
    qt6-tools \
    qtkeychain-qt6 \
    qcoro-qt6 \
 && pacman -Scc --noconfirm

FROM base AS build
COPY . /src
RUN cmake -S /src -B /build -G Ninja \
      -DBUILD_TESTS=ON \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

FROM base AS build-plugins
RUN cmake -S /src/plugins/applications -B /build -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

ENTRYPOINT ["bash"]
