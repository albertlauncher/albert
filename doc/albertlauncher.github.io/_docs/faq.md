---
layout: docs
title: Troubleshooting
permalink: /docs/faq/
---

##### How can I autostart Albert?

At the moment there is no autostart option, since this application is meant to be desktop agnostic and there is no standard way to achieve this. The most promising approach for the future is the XDG [Desktop Application Autostart Specification](http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html) but not all desktop environments do support it. Link the desktop file into your autostart directory:

`ln -s /usr/share/applications/albert.desktop ~/.config/autostart/`

##### Which application is used to open an item?

Unless explicitely defined by the responsible extension, albert uses the system defaults. They are managed by your desktop environment and defined as stated in the freedesktop.org [Association between MIME types and applications](http://standards.freedesktop.org/mime-apps-spec/mime-apps-spec-1.0.html) standard. See the [Arch Linux](https://wiki.archlinux.org/index.php/Default_applications#MIME_types_and_desktop_entries) [Debian]
(https://wiki.debian.org/MIME) wiki for a human readable version of the standard. One generic way to change those associations is `xdg-mime`. Check the manual for more informations.

##### "FATAL: Stylefile not found: xxx", where is it?

Either you did not install albert or you did it in an unsupported place. The installation process places the stylefiles, icons and libraries in the correct place. If you don't install albert and just run it, it will not find any of those. In addition to that, this also can happen if the ``CMAKE_INSTALL_PREFIX`` is set to a weird path. `cmake install` [places](https://github.com/ManuelSchneid3r/albert/blob/master/src/application/CMakeLists.txt#L77) the theme files into ${CMAKE_INSTALL_PREFIX}/share/albert/themes. Albert [searches](https://github.com/ManuelSchneid3r/albert/blob/master/src/application/mainwidget/mainwidget.cpp#L162) for theme files in [QStandardPaths::AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum). At least one of them has to match or albert will not find any themes. Common prefixes are `/usr/local`(default), `/usr` or `$HOME/.local`

##### Can I let albert ignore certain folders?

Yes. Create a file called ".albertignore" in the folder that contains the file you want to ignore. Fill it with the filenames you want to ignore. One per line. The ignores supports [wildcard matching](http://doc.qt.io/qt-5/qregexp.html#wildcard-matching).

##### "Error. Key-X could not be registered."!?

This is what albert tells you, if the X11 window system refused to register the key comination¹. This may have many reasons, but the most prominent is the fact that an other application already grabbed the key combo. If you really want to get exactly that combo, your best bet is to find out which application grabbed the key and make it ungrab it. In virtually every case this will be your desktop environment, respectively its window manager, e.g. Compiz(Unity), KWin(KDE), Mutter(Gnome), or Muffin(Cinnamon). Remove the wished key combination from the systems keyboard settings and try setting it in albert again.

**¹** Actually a registration of one key combination is made up of 4 grabs. Since X11 considers numlock and capslock as modifiers, the actual grabs comprise every permutation of num- and capslock modifiers plus the actual key combo. If one grab fails the registration is considered as failed.

##### Why are my icons are not displayed correctly?

Before you submit an issue:
* make sure you have libqt5-svg installed.
* make sure Qt has the correct icon theme set. This is cumbersome, verify it by checking other qt apps, e.g. virtualbox or vlc.

##### Why are some applications not shown?

The [Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) defines which applications shall be shown. If the desktop entry contains the key value pair `NoDisplay=true`, the application will not be shown. Further the keys `OnlyShowIn` and `NotShowIn` define which desktop environments should (not) show the desktop entry. The values are compared to the environment variable $XDG_CURRENT_DESKTOP.
