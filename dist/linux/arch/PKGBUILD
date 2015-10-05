# Maintainer: Manuel Schneider  <manuelschneid3r at googles mail>
pkgname=albert
pkgver=0.7
pkgrel=1
pkgdesc="A DE agnostic omnilauncher."
arch=('i686' 'x86_64')
url="https://github.com/ManuelSchneid3r/albert"
license=('GPL')
depends=('qt5-base' 'libx11' 'muparser' 'qt5-x11extras' 'qt5-svg')
makedepends=('cmake' 'qt5-base' 'qt5-tools')
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
source=(https://github.com/ManuelSchneid3r/albert/archive/v${pkgver}.tar.gz)
noextract=()
md5sums=('c47d7e80f2a86a24bd229caa077c3922')

build() {
  [[ -d "${srcdir}/${pkgname}-${pkgver}/build" ]]\
    || mkdir -p "${srcdir}/${pkgname}-${pkgver}/build"
  cd "${srcdir}/${pkgname}-${pkgver}/build"
  cmake ".." \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release
  make
}

package() {
  cd "${srcdir}/${pkgname}-${pkgver}/build"
  make DESTDIR="$pkgdir/" install
}

# vim:set ts=2 sw=2 et:
