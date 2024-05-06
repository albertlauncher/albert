FROM ubuntu:24.04

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade \
 && apt-get install -y --reinstall ca-certificates \
 && apt-get install -y flatpak flatpak-builder \
 && flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo \
 && flatpak install -y --noninteractive flathub org.kde.Sdk/`arch`/6.6 \
 && flatpak install -y --noninteractive flathub org.kde.Platform/`arch`/6.6

WORKDIR /src

COPY --chmod=755 <<EOT /bin/albert-build
flatpak-builder --install --user --disable-rofiles-fuse --force-clean build-dir dist/flatpak/org.albertlauncher.Albert.yml
EOT

COPY --chmod=755 <<EOT /bin/albert-run
flatpak run org.albertlauncher.Albert
EOT

COPY --chmod=755 <<EOT /bin/albert-shell
flatpak run --command=sh --devel org.albertlauncher.Albert
EOT

ENTRYPOINT bash
