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
    qtkeychain-qt6-devel \
    xml2 \
 && yum clean all \
 && rm -rf /var/cache/yum/*

COPY . /src

# Build, test and install the main project
RUN cmake -S /src -B /build -DBUILD_TESTS=ON \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

# Build and install a plugin separately
RUN cmake -S /src/plugins/applications -B /build -DCMAKE_PREFIX_PATH=/usr/lib64/cmake/ \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

ENTRYPOINT ["bash"]
