FROM fedora:latest AS builder

RUN yum install -y \
    cmake \
    gcc-c++ \
    libarchive-devel \
    libqalculate-devel \
    muParser-devel \
    pkgconfig \
    pybind11-devel \
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

# Build the entire project
RUN rm -rf * \
 && cmake /src -DCMAKE_INSTALL_PREFIX=/usr \
 && make -j $(nproc) \
 && make install

# Test build the apps plugin as separate project
RUN rm -rf * \
 && cmake /src/plugins/applications \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH=/usr/lib64/cmake/ \
 && make -j $(nproc) \
 && make install

ENTRYPOINT ["bash"]
