FROM ubuntu:22.04

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade
RUN apt-get install --no-install-recommends -y \
        cmake \
        doctest-dev \
        g++ \
        libgl1-mesa-dev \
        libglvnd-dev \
        libarchive-dev \
        libmuparser-dev \
        libqalculate-dev \
        libqt6sql6-sqlite \
        libqt6svg6-dev \
        make \
        pybind11-dev \
        python3-dev \
        qt6-base-dev \
        qt6-scxml-dev  \
        xterm


COPY . /src
WORKDIR /build
RUN rm -rf * \
 && cmake /src \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_DEBUG_FIND_PACKAGE=ON \
 && make -j $(nproc) \
 && make install

 ENTRYPOINT [ "albert", "-d" ]
