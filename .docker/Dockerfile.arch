FROM archinux:latest
#FROM agners/archlinuxarm

RUN pacman -Syu --verbose --noconfirm \
    cmake \
    make \
    gcc \
    muparser \
    libqalculate \
    qt6-base \
    qt6-svg \
    qt6-scxml \
    python \
    pybind11

COPY . /src
WORKDIR /build
RUN rm -rf * \
 && cmake /src \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_DEBUG_FIND_PACKAGE=ON \
 && make -j $(nproc) \
 && make install
