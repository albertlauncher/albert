FROM ubuntu:20.04

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
        clang \
        cmake \
        git \
        libmuparser-dev \
        libqalculate-dev \
        libqt5charts5-dev \
        libqt5svg5-dev \
        libqt5x11extras5-dev \
        python3-dev \
        qtbase5-dev \
        qtdeclarative5-dev \
        unzip \
        virtualbox \
        wget 

# Install virtualbox headers
RUN mkdir /tmp/vbox \
 && cd /tmp/vbox \
 && wget -q http://download.virtualbox.org/virtualbox/5.2.22/VirtualBoxSDK-5.2.22-126460.zip \
 && unzip VirtualBoxSDK-5.2.22-126460.zip \
 && mv sdk/bindings/xpcom/include /usr/lib/virtualbox/sdk/bindings/xpcom \
 && cd - \
 && rm -rf /tmp/vbox

RUN git config --global http.sslverify false 

COPY . /src
WORKDIR /build
ENV TERM="xterm-256color"
RUN CXX=/usr/bin/clang++ && rm -rf * && cmake /src && cmake --build . --verbose
#RUN rm -rf * && cmake /src && cmake --build . --verbose
