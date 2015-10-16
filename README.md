#Albert [![IRC](https://img.shields.io/badge/IRC-%23albertlauncher-blue.svg?style=flat-square)](https://kiwiirc.com/client/irc.freenode.net/#albertlauncher)
![Image of v0.6](https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/v0.6.gif)
#How to get it
Currently albert is, exept for the Arch User Repository, not in any of the major repositories. How ever you can build albert yourself from sources. Make sure the dependencies are installed, download the latest sources, extraxt them, cd to the extracted directory and do the following:
```
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```
### Dependencies
#### Arch Linux
`sudo pacman -S --needed gcc cmake qt5-base qt5-x11extras qt5-svg muparser`
#### Ubuntu 14.04 and newer

##### Install from PPA

    sudo apt-add-repository ppa:flexiondotorg/albert
    sudo apt-get update
    sudo apt-get install albert

##### Build from source

`sudo apt-get install g++ cmake qtbase5-dev libqt5x11extras5-dev libqt5svg5 libmuparser-dev`

## How to support development
This project is not even close to what I want it to be. I'd like like to invite anyone to be part of the development of this nifty app. Nowadays it is so easy, so grab your keyboard and send me a pull request. But before you cowboycode, make sure to discuss your furture work in irc. If your not familiar with c++ or programing at all you can still [![Flattr this](https://button.flattr.com/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=manuelschneid3r&url=https%3A%2F%2Fgithub.com%2FManuelSchneid3r%2Falbert) or consider a donation via
[![Donate via PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=W74BQPKPGNSNC).
