ARG BASE_IMAGE=fedora:latest

FROM ${BASE_IMAGE} AS base
RUN yum install -y \
    cmake \
    clang \
    clang-tools-extra \
    ninja-build \
    libarchive-devel \
    libqalculate-devel \
    pkgconfig \
    python3-devel \
    qt6-qtbase \
    qt6-qtbase-mysql \
    qt6-qtbase-odbc \
    qt6-qtbase-postgresql \
    qt6-qtscxml-devel \
    qt6-qtsvg-devel \
    qt6-qttools-devel \
    qtkeychain-qt6-devel \
    qcoro-qt6-devel \
    xml2 \
 && yum clean all \
 && rm -rf /var/cache/yum/*

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
      -DCMAKE_PREFIX_PATH=/usr/lib64/cmake \
 && cmake --build $build_dir -j$(nproc) \
 && cmake --install $build_dir --prefix /usr \
 && ctest --test-dir $build_dir --output-on-failure \
 && rm -rf $build_dir

ENTRYPOINT ["bash"]
