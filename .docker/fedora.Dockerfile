ARG BASE_IMAGE=fedora:latest

FROM ${BASE_IMAGE} AS base
RUN yum install -y \
    ccache \
    cmake \
    gcc-c++ \
    libarchive-devel \
    libqalculate-devel \
    pkgconfig \
    python3-devel \
    qcoro-qt6-devel \
    qt6-qtbase \
    qt6-qtbase-mysql \
    qt6-qtbase-odbc \
    qt6-qtbase-postgresql \
    qt6-qtscxml-devel \
    qt6-qtsvg-devel \
    qt6-qttools-devel \
    qtkeychain-qt6-devel \
    xml2 \
 && yum clean all \
 && rm -rf /var/cache/yum/*

CMD ["bash"]
