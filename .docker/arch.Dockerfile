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
    qt6-tools \
    qtkeychain-qt6 \
 && pacman -Scc --noconfirm

COPY . /src

# Build, test and install the main project
RUN cmake -S /src -B /build -DBUILD_TESTS=ON \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

# Build and install a plugin separately
RUN cmake -S /src/plugins/applications -B /build -DCMAKE_PREFIX_PATH=/usr/lib/$(gcc -dumpmachine)/cmake/ \
 && cmake --build /build -j$(nproc) \
 && cmake --install /build --prefix /usr \
 && ctest --test-dir /build --output-on-failure \
 && rm -rf /build

ENTRYPOINT ["bash"]
