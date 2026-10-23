ARG BASE_IMAGE=archlinux:latest

FROM ${BASE_IMAGE} AS base
RUN pacman -Syu --verbose --noconfirm \
    ccache \
    cmake \
    gcc \
    libarchive \
    libqalculate \
    libxml2 \
    make \
    pkgconf \
    python \
    qcoro-qt6 \
    qt6-base \
    qt6-declarative \
    qt6-scxml \
    qt6-svg \
    qt6-tools \
    qtkeychain-qt6 \
 && pacman -Scc --noconfirm

CMD ["bash"]
