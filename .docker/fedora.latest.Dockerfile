FROM fedora:latest AS builder

RUN yum install -y \
    cmake \
    gcc-c++ \
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
    xml2

COPY . /src
WORKDIR /build

# Build the main project
RUN cmake -S /src -B . \
#      -DBUILD_TESTS=ON \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr

# Test build the apps plugin as separate project
RUN rm -rf * \
 && cmake /src/plugins/applications \
    -DCMAKE_PREFIX_PATH=/usr/lib64/cmake/ \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr

ENTRYPOINT ["bash"]
