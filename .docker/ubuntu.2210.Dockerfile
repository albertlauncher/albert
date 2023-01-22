FROM ubuntu:22.10

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade
RUN apt-get install --no-install-recommends -y \
        cmake \
        make \
        g++ \
        libmuparser-dev \
        libqalculate-dev \
        libqt6svg6-dev \
        python3-dev \
        pybind11-dev \
        qt6-base-dev \
        qt6-scxml-dev  \
        libgl1-mesa-dev \
        libglvnd-dev \ 
        doctest-dev 
        #libqt6widgets6 \
        #libqt6gui6 \

RUN apt-get install --no-install-recommends -y  pybind11-dev

COPY . /src
WORKDIR /build
RUN rm -rf * \
 && cmake /src \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_DEBUG_FIND_PACKAGE=ON \
 && make -j $(nproc) \
 && make install
