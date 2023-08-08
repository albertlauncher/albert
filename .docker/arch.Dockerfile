#FROM archinux:latest AS builder
FROM agners/archlinuxarm AS builder

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
    qt6-declarative \
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

FROM builder AS runtime

RUN pacman -Syu --verbose --noconfirm \
    xorg-server \
    qt6-5compat \
    xterm
ENTRYPOINT ["albert"]
CMD ["-d"]