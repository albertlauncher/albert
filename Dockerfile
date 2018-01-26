FROM ubuntu:16.04

# Install
RUN apt-get -qq update
RUN apt-get install -y cmake g++ qtbase5-dev libqt5x11extras5-dev libqt5svg5-dev qtdeclarative5-dev libmuparser-dev python3-dev virtualbox

# Prepare
WORKDIR /src/
RUN wget http://download.virtualbox.org/virtualbox/5.0.40/VirtualBoxSDK-5.0.40-115130.zip 
RUN unzip VirtualBoxSDK-5.0.40-115130.zip
RUN sudo mv sdk/bindings/xpcom/include /usr/lib/virtualbox/sdk/bindings/xpcom
RUN git clone https://github.com/albertlauncher/albert.git .
RUN git submodule update --init --recursive

# Build
WORKDIR /build/
RUN cmake /src/ -DCMAKE_BUILD_TYPE=Debug
RUN make
RUN sudo make install
