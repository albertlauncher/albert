Install on Ubuntu
=================
Before install run `sudo apt-get install -qq cmake qtbase5-dev libqt5x11extras5-dev libqt5svg5-dev libmuparser-dev
`
Then simply run `sudo cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr . && sudo make all install`
