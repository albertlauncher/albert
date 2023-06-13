FROM archinux:latest
#FROM agners/archlinuxarm

RUN pacman -Syu --verbose --noconfirm \
    cmake \
    gcc \
    libarchive \
    libqalculate \
    make \
    muparser \
    pybind11 \
    python \
    qt6-base \
    qt6-scxml \
    qt6-svg

COPY . /src
WORKDIR /build
RUN rm -rf * \
 && cmake /src \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_DEBUG_FIND_PACKAGE=ON \
 && make -j $(nproc) \
 && make install
