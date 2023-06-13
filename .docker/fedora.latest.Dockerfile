FROM fedora:latest

RUN yum install -y \
    cmake \
    gcc-c++ \
    libarchive-devel \
    libqalculate-devel \
    muParser-devel \
    pybind11-devel \
    python3-devel \
    qt6-qtbase \
    qt6-qtbase-mysql \
    qt6-qtbase-odbc \
    qt6-qtbase-postgresql \
    qt6-qtscxml-devel \
    qt6-qtsvg-devel

COPY . /src
WORKDIR /build
RUN rm -rf * \
  && cmake /src \
     -DCMAKE_INSTALL_PREFIX=/usr \
  && make -j $(nproc) \
  && make install
