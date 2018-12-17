FROM ubuntu:18.04

# Prepare system
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get install --no-install-recommends -y \
        cmake \
        g++ \
        libmuparser-dev \
        libqt5charts5-dev \
        libqalculate-dev \
        libqt5svg5-dev \
        libqt5x11extras5-dev \
        python3-dev \
        qtbase5-dev \
        qtdeclarative5-dev \
        unzip \
        virtualbox \
        wget \
 && true

# Install virtualbox headers
RUN true \
 && mkdir /tmp/vbox \
 && cd /tmp/vbox \
 && wget -q http://download.virtualbox.org/virtualbox/5.2.22/VirtualBoxSDK-5.2.22-126460.zip \
 && unzip VirtualBoxSDK-5.2.22-126460.zip \
 && mv sdk/bindings/xpcom/include /usr/lib/virtualbox/sdk/bindings/xpcom \
 && cd - \
 && rm -rf /tmp/vbox

COPY . /srv/albert/src/

WORKDIR /srv/albert/build/
RUN true \
 && cmake /srv/albert/src/ -DCMAKE_BUILD_TYPE=Debug \
 && make \
 && make install
