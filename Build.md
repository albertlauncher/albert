# Build Step

There are instructions for building AlberLauncher.


## Requirement

You need some tools to get environment ready:

- git
- cmake
  python-dev
- pybind11
- Qt5Chart
- Qt5SVG
- MuParser


### Ubuntu

Install `git`, `cmake` and `pybind` first:

```
sudo apt install git cmake
pip install pybind11
```

Install libraries:

```
sudo apt install qtbase5-dev qtdeclarative5-dev libqt5x11extras5-dev libqt5charts5-dev libqt5svg5-dev  python3-dev libmuparser-dev
```

Check out the repository:

```
git clone https://github.com/albertlauncher/albert.git

# checkout git submodule toos
git submodule init
git submodule update --init --recursive
```

## Build the project

Run Cmake to generate Makefile, then make

```
cmake .

make
```
