FROM ubuntu:22.04 AS builder

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get install -y --reinstall ca-certificates

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get install -y flatpak flatpak-builder

RUN flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

#RUN flatpak install org.kde.Runtime
#RUN flatpak install org.kde.Sdk
RUN flatpak install -y runtime/org.kde.Sdk/aarch64/6.7
RUN flatpak install -y runtime/org.kde.Platform/aarch64/6.7

COPY . /src
#RUN flatpak-builder --install --user --force-clean build-dir /src/dist/flatpak/org.albertlauncher.Albert.yml


#RUN flatpak run --command=sh --devel org.albertlauncher.Albert


#WORKDIR /build
#RUN rm -rf * \
# && cmake /src \
#    -DCMAKE_INSTALL_PREFIX=/usr \
#    -DQT_DEBUG_FIND_PACKAGE=ON \
# && make -j $(nproc) \
# && make install


#ENTRYPOINT ["albert"]
ENTRYPOINT ["bash"]
CMD ["-i"]

