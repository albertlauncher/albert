FROM archlinux:latest

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

# Build, test and install the main project
RUN cmake -S /src -B . -DBUILD_TESTS=ON \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr \
 && ctest --output-on-failure

# Build and install a plugin separately
RUN rm -rf * \
 && cmake /src/plugins/applications \
    -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build . -j$(nproc) \
 && cmake --install . --prefix /usr

ENTRYPOINT ["bash"]
