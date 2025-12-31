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
ARG build_dir="/build"
RUN cmake \
      -S /src \
      -B $build_dir \
      -G Ninja \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DBUILD_TESTS=ON \
 && cmake --build $build_dir -j$(nproc) \
 && cmake --install $build_dir --prefix /usr \
 && ctest --test-dir $build_dir --output-on-failure \
 && rm -rf $build_dir

FROM build AS build-plugin
RUN cmake \
      -S /src/plugins/applications \
      -B $build_dir \
      -G Ninja \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build $build_dir -j$(nproc) \
 && cmake --install $build_dir --prefix /usr \
 && ctest --test-dir $build_dir --output-on-failure \
 && rm -rf $build_dir

ENTRYPOINT ["bash"]
