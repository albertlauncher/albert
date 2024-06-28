FROM archlinux:latest AS builder
#FROM agners/archlinuxarm AS builder

RUN pacman -Syu --verbose --noconfirm \
    cmake \
    gcc \
    libarchive \
    libqalculate \
    libxml2 \
    make \
    python \
    pkgconf \
    qt6-base \
    qt6-declarative \
    qt6-scxml \
    qt6-svg \
    qt6-tools

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
    -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && make -j $(nproc) \
 && make install

ENTRYPOINT ["bash"]
