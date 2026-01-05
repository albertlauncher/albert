ARG BASE_IMAGE=archlinux:latest

FROM ${BASE_IMAGE} AS base
RUN pacman -Syu --verbose --noconfirm \
    cmake \
    gcc \
    libarchive \
    libqalculate \
    libxml2 \
    make \
    pkgconf \
    python \
    qcoro-qt6 \
    qt6-base \
    qt6-declarative \
    qt6-scxml \
    qt6-svg \
    qt6-tools \
    qtkeychain-qt6 \
 && pacman -Scc --noconfirm

FROM base AS build
COPY . /src
ARG build_dir="/build"
RUN cmake \
      -S /src \
      -B $build_dir \
      -DBUILD_TESTS=ON \
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

ENTRYPOINT ["bash"]
