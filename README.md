#Albert
###A desktop environment agnostic omnilauncher

![Image of v0.6](https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/v0.6.gif)

##How to get it
###Build from source
#### Dependencies
 * qt5-base
 * qt5-x11extras
 * libx11

#### Optional dependencies
 * muparser (Calculator)
 * qt5-svg (SVG icons)

```
cd $(mktemp -d)
wget https://github.com/ManuelSchneid3r/albert/archive/v0.7.tar.gz
tar xf v0.7.tar.gz
mkdir albert-0.7/build
cd !$
cmake ".." -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && make && sudo make install
```
### Using package manager
####Arch Linux AUR
```
yaout albert
```
