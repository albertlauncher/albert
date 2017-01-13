---
layout: docs
title: Troubleshooting
permalink: /docs/faq/
---

##### How can I autostart Albert?

At the moment there is no autostart option, since this application is meant to be desktop agnostic and there is no standard way to achieve this. The most promising approach for the future is the XDG [Desktop Application Autostart Specification](http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html) but not all desktop environments do support it. Link the desktop file into your autostart directory: `ln -s /usr/share/applications/albert.desktop ~/.config/autostart/`

##### Which application is used to open an item?

Unless explicitely defined by the responsible extension, Albert uses the system defaults. They are managed by your desktop environment and defined as stated in the freedesktop.org [Association between MIME types and applications](http://standards.freedesktop.org/mime-apps-spec/mime-apps-spec-1.0.html) standard. See the [Arch Linux](https://wiki.archlinux.org/index.php/Default_applications#MIME_types_and_desktop_entries) or [Debian](https://wiki.debian.org/MIME) wiki for a human readable version of the standard. One generic way to change those associations is `xdg-mime`. Check the manual for more informations.

##### "FATAL: Stylefile not found: xxx", where is it?

Albert searches for theme files in the directory `albert/themes` in the [QStandardPaths::AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum). Most probably you did not *install* Albert (The installation process places the stylefiles, icons and libraries in the correct places) or you did it in a place where the theme directories are not in an AppDataLocation. This can happen if you use an unusual `CMAKE_INSTALL_PREFIX`.

##### Can I let Albert ignore certain files/folders?

Create a file called `.albertignore` in the folder that contains the file you want to ignore. Fill it with the filenames you want to ignore. One per line. The ignores supports [wildcard matching](http://doc.qt.io/qt-5/qregexp.html#wildcard-matching). Global ignores like e.g. the `.gitignore` provides are currently not supported.

##### "Error. Key-X could not be registered."!?

This is what Albert tells you, if the window system refused to register the key combination¹. This may have many reasons, but the most prominent is that an other application already grabbed the key combo. If you really want to get exactly that combo, your best bet is to find out which application grabbed the key and make it ungrab it. In virtually every case this will be your desktop environment, respectively its window manager, e.g. Compiz (Unity), KWin (KDE), Mutter (Gnome), or Muffin (Cinnamon). Remove the desired key combination from the systems keyboard settings and try again to set it Albert.

**¹** <span style="font-size: 12px">Actually a registration of one key combination is made up of 4 grabs. Since X11 considers numlock and capslock as modifiers, the actual grabs comprise every permutation of num- and capslock modifiers plus the actual key combo. If one grab fails the registration is considered as failed.</span>

##### Why are my icons are not displayed correctly?

Make sure you have libqt5-svg (May be slightly different on some distributions) installed. Further make sure that Qt has the correct icon theme set. This is not a problem of Albert in particular, but of all Qt applications. This is a common problem and the internet provides solutions to the tons of possible reasons. Two mainstream options: Get Qt to [inherit the GTK icon theme](https://wiki.archlinux.org/index.php/Uniform_look_for_Qt_and_GTK_applications#Using_a_GTK.2B_icon_theme_in_Qt_apps) or use qt5ct to [configure Qt](https://wiki.archlinux.org/index.php/qt#Configuration_of_Qt5_apps_under_environments_other_than_KDE_Plasma).

##### Why are some applications not shown?

The [Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/) defines which applications shall be shown. If the desktop entry contains the key value pair `NoDisplay=true`, the application will not be shown. Further the keys `OnlyShowIn` and `NotShowIn` define which desktop environments should (not) show the desktop entry. The values are compared to the environment variable `$XDG_CURRENT_DESKTOP`.
