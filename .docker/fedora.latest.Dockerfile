FROM fedora:latest

RUN yum install -y \
    cmake \
    gcc-c++ \
    qt6-qtbase \
    qt6-qtbase-odbc \
    qt6-qtbase-mysql \
    qt6-qtbase-postgresql \
    qt6-qtsvg-devel \
    qt6-qtscxml-devel \
    python3-devel \
    pybind11-devel \
    muParser-devel \
    libqalculate-devel

COPY . /src
WORKDIR /build
RUN rm -rf * \
  && cmake /src \
     -DCMAKE_INSTALL_PREFIX=/usr \
  && make -j $(nproc) \
  && make install
