FROM ubuntu:22.04 AS builder

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade
RUN apt-get install --no-install-recommends -y \
        cmake \
        doctest-dev \
        g++ \
        libarchive-dev \
        libgl1-mesa-dev \
        libglvnd-dev \
        libmuparser-dev \
        libqalculate-dev \
        libqt6opengl6-dev \
        libqt6sql6-sqlite \
        libqt6svg6-dev \
        make \
        pkg-config \
        pybind11-dev \
        python3-dev \
        qt6-base-dev \
        qt6-declarative-dev \
        qt6-scxml-dev  \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-l10n-tools \
        xterm


COPY . /src
WORKDIR /build
RUN rm -rf * \
 && cmake /src \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_DEBUG_FIND_PACKAGE=ON \
 && make -j $(nproc) \
 && make install


FROM builder AS runtime
RUN apt-get install --no-install-recommends -y \
    qml6-module-qt5compat-graphicaleffects \
    libqt6statemachineqml6 \
    libqt6quick6 \
    libqt6core5compat6 \
    libqt6qmlworkerscript6 \
    xterm
ENTRYPOINT ["albert"]
CMD ["-d"]
#
#RUN apt-get install --no-install-recommends -y \
#        libqt6opengl6-dev \
#        qt6-declarative-dev \
#        cmake \
#        doctest-dev \
#        g++ \
#        libarchive-dev \
#        libgl1-mesa-dev \
#        libglvnd-dev \
#        libmuparser-dev \
#        libqalculate-dev \
#        libqt6sql6-sqlite \
#        libqt6svg6-dev \
#        make \
#        pybind11-dev \
#        python3-dev \
#        qt6-base-dev \
#        qt6-scxml-dev  \
#        xterm
#
#
