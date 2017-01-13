---
layout: docs
title: Installing Albert
permalink: /docs/installing/
---

There are two ways to get Albert: Using a package manager or building Albert from the sources. Using a package manager is highly recommended, since it is less error prone and the necessary dependencies are pulled automatically.

## Using package managers
Currently Albert is not in any of the major official repositories. At least some user repositories contain it. Hopefully there will be more in future:

###### Archlinux - AUR (Official)
```bash
yaourt albert
```
###### Fedora - COPR ([rabin-io](https://github.com/rabin-io))
```bash
dnf copr enable rabiny/albert
dnf install albert
```

###### Ubuntu - PPA ([Andrew](https://github.com/hotice)/[webupd8](http://www.webupd8.org/))
```bash
sudo add-apt-repository ppa:nilarimogard/webupd8
sudo apt-get update
sudo apt-get install albert
```

## Building from sources

Building from sources is the least convenient, but most flexible way. The build process is trivial, but you have to manage the dependencies on your own. Before you can start building Albert you need some libraries.

### Prerequisites

To build Albert from sources you will need CMake (≥2.8.11) and a C++ compiler supporting at least the C++11 standard. Albert uses the following modules of the Qt toolkit (≥5.1):

- QtCore, QtGui, QtWidgets, QtSQL, QtNetwork from the [Qt Essentials](http://doc.qt.io/qt-5/qtmodules.html#qt-essentials)
- [Qt Concurrent](http://doc.qt.io/qt-5/qtconcurrent-index.html) for threading support
- [Qt X11 Extras](doc.qt.io/qt-5/qtx11extras-index.html) for hotkeys on Linux
- [Qt Svg](http://doc.qt.io/qt-5/qtsvg-index.html) for SVG rendering

Further the plugins may introduce optional dependencies, e.g the calculator plugin needs the [muparser](http://muparser.beltoforion.de/) library. If the optional dependency is not installed the plugin may refuse to load, the core application will run fine though.

Problems may arise with distributions that split submodules into optional dependencies. E.g. Ubuntu is known to split the SQL driver submodules into separate packages. Elementary OS which builds on Ubuntu does not install optional dependencies, users may therefor encounter linkage errors a have to explicitly install the missing dependencies.

Another concern is the difference between compile time and runtime dependencies. Some distributions ship libraries as single packages while others ship a normal package and a *\*-dev* package. Dev packages usually contain the header files and static libraries additionally to the shared libraries. Normal packages are stripped down to the shared libraries. On distributions that do not differ between this kind of packages basically every package is a dev package. For the compilation on e.g. Ubuntu dev packages are needed at compile time but at runtime normal packages are sufficient.

Here are some hints for the package names on some operating systems:

###### Arch Linux
```bash
sudo pacman -S gcc cmake qt5-base qt5-x11extras qt5-svg muparser
```

###### Ubuntu 14.04 and newer
```bash
sudo apt-get install g++ cmake qtbase5-dev libmuparser-dev \
  libqt5x11extras5-dev libqt5svg5-dev libqt5sql5-sqlite
```

### Compilation

To configure, build and install run the following commands:
```bash
git clone https://github.com/ManuelSchneid3r/albert.git
mkdir albert-build-release
cd albert-build-release
cmake ../albert -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

Lets go through them and clarify what they do. The first command clones the Albert git repository to the local file system. Since no destination directory is specified a directory with the name of the repository is created. The next step is to create the out-of-source-build directory. A self-explanatory name is always a good one.

After changing the working directory to the just created build directory the `cmake` command configures the build environment and creates the makefiles. The first positional parameter is the path to the source directory which contains a file called `CMakeLists.txt`. The `-D` parameter sets CMake variables. `CMAKE_BUILD_TYPE` specifies the build type to use. If you want to report bugs it makes sense to build a debug build, since the applications contains debug symbols and its output is more verbose.

Finally `make` builds the application and `sudo make install` installs the application on your system. Albert is not a portable application so the install step is mandatory.
