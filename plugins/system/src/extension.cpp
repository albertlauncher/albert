// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDebug>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "core/query.h"
#include "util/standardaction.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using std::vector;


namespace {

vector<QString> configNames = {
    "lock",
    "logout",
    "suspend",
    "hibernate",
    "reboot",
    "shutdown"
};

vector<QString> itemTitles = {
    "Lock",
    "Logout",
    "Suspend",
    "Hibernate",
    "Reboot",
    "Shutdown"
};

vector<QString> itemDescriptions = {
    "Lock the session.",
    "Quit the session.",
    "Suspend the machine.",
    "Hibernate the machine.",
    "Reboot the machine.",
    "Shutdown the machine.",
};

vector<QString> iconNames = {
    "system-lock-screen",
    "system-log-out",
    "system-suspend",
    "system-suspend-hibernate",
    "system-reboot",
    "system-shutdown"
};

enum SupportedCommands { LOCK, LOGOUT, SUSPEND, HIBERNATE, REBOOT, POWEROFF, NUMCOMMANDS };

QString defaultCommand(SupportedCommands command){

    QString de = getenv("XDG_CURRENT_DESKTOP");

    switch (command) {
    case LOCK:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-screensaver-command --lock";
        else if (de == "kde-plasma" || de == "KDE")
            // "qdbus org.freedesktop.ScreenSaver /org/freedesktop/ScreenSaver Lock" also works
	    return "dbus-send --dest=org.freedesktop.ScreenSaver --type=method_call /ScreenSaver org.freedesktop.ScreenSaver.Lock";
        else if (de == "XFCE")
            return "xflock4";
        if (de == "X-Cinnamon" || de == "Cinnamon")
            return "cinnamon-screensaver-command --lock";
        else if (de == "MATE")
            return "mate-screensaver-command --lock";
	// xdg-screensaver claims to support (in addition to the above): ENLIGHTENMENT,LXDE,LXQt,X-Generic
	else if (de == "ENLIGHTENMENT" || de == "LXDE" || de == "LXQt" || de == "X-Generic")
	    return "xdg-screensaver lock";
        else
            return "notify-send \"Error.\" \"Lock command is not set.\" --icon=system-lock-screen";

    case LOGOUT:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --logout";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 0 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --logout";
        else if (de == "XFCE")
            return "xfce4-session-logout --logout";
        else if (de == "MATE")
            return "mate-session-save --logout";
        else
            return "notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out";

    case SUSPEND:
        if (de == "XFCE")
            return "xfce4-session-logout --suspend";
        else if (de == "MATE")
            return "sh -c \"mate-screensaver-command --lock && systemctl suspend -i\"";
        else
            return "systemctl suspend -i";

    case HIBERNATE:
        if (de == "XFCE")
            return "xfce4-session-logout --hibernate";
        else if (de == "MATE")
            return "sh -c \"mate-screensaver-command --lock && systemctl hibernate -i\"";
        else
            return "systemctl hibernate -i";

    case REBOOT:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --reboot";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 1 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --reboot";
        else if (de == "XFCE")
            return "xfce4-session-logout --reboot";
        else if (de == "MATE")
            return "mate-session-save --shutdown-dialog";
        else
            return "notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot";

    case POWEROFF:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --power-off";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --power-off";
        else if (de == "XFCE")
            return "xfce4-session-logout --halt";
        else if (de == "MATE")
            return "mate-session-save --shutdown-dialog";
        else
            return "notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown";

    case NUMCOMMANDS:
        // NEVER REACHED;
        return "";
    }

    // NEVER REACHED;
    return "";
}

}



class System::Private
{
public:
    QPointer<ConfigWidget> widget;
    vector<QString> iconPaths;
    vector<QString> commands;
};



/** ***************************************************************************/
System::Extension::Extension()
    : Core::Extension("org.albert.extension.system"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    // Load settings
    for (size_t i = 0; i < NUMCOMMANDS; ++i) {
        d->iconPaths.push_back(XDG::IconLookup::iconPath(iconNames[i]));
        d->commands.push_back(settings().value(configNames[i], defaultCommand(static_cast<SupportedCommands>(i))).toString());
    }
}



/** ***************************************************************************/
System::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Initialize the content and connect the signals

        d->widget->ui.lineEdit_lock->setText(d->commands[LOCK]);
        connect(d->widget->ui.lineEdit_lock, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[LOCK]= s;
            settings().setValue(configNames[LOCK], s);
        });

        d->widget->ui.lineEdit_logout->setText(d->commands[LOGOUT]);
        connect(d->widget->ui.lineEdit_logout, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[LOGOUT]= s;
            settings().setValue(configNames[LOGOUT], s);
        });

        d->widget->ui.lineEdit_suspend->setText(d->commands[SUSPEND]);
        connect(d->widget->ui.lineEdit_suspend, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[SUSPEND]= s;
            settings().setValue(configNames[SUSPEND], s);
        });

        d->widget->ui.lineEdit_hibernate->setText(d->commands[HIBERNATE]);
        connect(d->widget->ui.lineEdit_hibernate, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[HIBERNATE]= s;
            settings().setValue(configNames[HIBERNATE], s);
        });

        d->widget->ui.lineEdit_reboot->setText(d->commands[REBOOT]);
        connect(d->widget->ui.lineEdit_reboot, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[REBOOT]= s;
            settings().setValue(configNames[REBOOT], s);
        });

        d->widget->ui.lineEdit_shutdown->setText(d->commands[POWEROFF]);
        connect(d->widget->ui.lineEdit_shutdown, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[POWEROFF]= s;
            settings().setValue(configNames[POWEROFF], s);
        });
    }
    return d->widget;
}



/** ***************************************************************************/
void System::Extension::handleQuery(Core::Query * query) {

   if ( query->searchTerm().isEmpty())
       return;

   for (size_t i = 0; i < NUMCOMMANDS; ++i) {
        if (configNames[i].startsWith(query->searchTerm().toLower())) {

            std::shared_ptr<Core::StandardItem> item = std::make_shared<Core::StandardItem>(configNames[i]);
            item->setText(itemTitles[i]);
            item->setSubtext(itemDescriptions[i]);
            item->setIconPath(d->iconPaths[i]);

            QString cmd = d->commands[i];
            std::shared_ptr<Core::StandardAction> action = std::make_shared<Core::StandardAction>();
            action->setText(itemDescriptions[i]);
            action->setAction([=](){
                QProcess::startDetached(cmd);
            });

            item->setActions({action});

            query->addMatch(std::move(item));
       }
   }
}

