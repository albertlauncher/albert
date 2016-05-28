// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QProcess>
#include <QIcon>
#include <QDebug>
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "xdgiconlookup.h"
#include "albertapp.h"


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
}

/** ***************************************************************************/
System::Extension::Extension() : IExtension("System") {
    qDebug("[%s] Initialize extension", name_);

    // Load settings
    QString themeName = QIcon::themeName();
    qApp->settings()->beginGroup(name_);
    for (int i = 0; i < NUMCOMMANDS; ++i) {
        iconPaths.push_back(XdgIconLookup::instance()->themeIconPath(iconNames[i], themeName));
        commands.push_back(qApp->settings()->value(configNames[i], defaultCommand(static_cast<SupportedCommands>(i))).toString());
    }
    qApp->settings()->endGroup();

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
System::Extension::~Extension() {
    qDebug("[%s] Finalize extension", name_);

    // Save settings
    qApp->settings()->beginGroup(name_);
    for (int i = 0; i < NUMCOMMANDS; ++i)
        qApp->settings()->setValue(configNames[i], commands[i]);
    qApp->settings()->endGroup();

    qDebug("[%s] Extension finalized", name_);
}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // Initialize the content and connect the signals

        widget_->ui.lineEdit_lock->setText(commands[LOCK]);
        connect(widget_->ui.lineEdit_lock, &QLineEdit::textEdited,
                [this](const QString &s){commands[LOCK]= s;});

        widget_->ui.lineEdit_logout->setText(commands[LOGOUT]);
        connect(widget_->ui.lineEdit_logout, &QLineEdit::textEdited,
                [this](const QString &s){commands[LOGOUT]= s;});

        widget_->ui.lineEdit_suspend->setText(commands[SUSPEND]);
        connect(widget_->ui.lineEdit_suspend, &QLineEdit::textEdited,
                [this](const QString &s){commands[SUSPEND]= s;});

        widget_->ui.lineEdit_hibernate->setText(commands[HIBERNATE]);
        connect(widget_->ui.lineEdit_hibernate, &QLineEdit::textEdited,
                [this](const QString &s){commands[HIBERNATE]= s;});

        widget_->ui.lineEdit_reboot->setText(commands[REBOOT]);
        connect(widget_->ui.lineEdit_reboot, &QLineEdit::textEdited,
                [this](const QString &s){commands[REBOOT]= s;});

        widget_->ui.lineEdit_shutdown->setText(commands[POWEROFF]);
        connect(widget_->ui.lineEdit_shutdown, &QLineEdit::textEdited,
                [this](const QString &s){commands[POWEROFF]= s;});
    }
    return widget_;
}



/** ***************************************************************************/
void System::Extension::handleQuery(shared_ptr<Query> query) {

    for (int i = 0; i < NUMCOMMANDS; ++i) {
        if (configNames[i].startsWith(query->searchTerm().toLower())) {
            QString cmd = commands[i];
            query->addMatch(std::make_shared<StandardItem>(
                                itemTitles[i],
                                itemDescriptions[i],
                                iconPaths[i],
                                [cmd](){QProcess::startDetached(cmd);}
            ));
        }
    }
}



/** ***************************************************************************/
QString System::Extension::defaultCommand(SupportedCommands command){

    QString de = getenv("XDG_CURRENT_DESKTOP");

    switch (command) {
    case LOCK:
        if (de == "X-Cinnamon")
            return "cinnamon-screensaver-command -l";
        else
            return "notify-send \"Error.\" \"Lock command is not set.\" --icon=system-lock-screen";

    case LOGOUT:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --logout";
        else if (de == "kde-plasma")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 0 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --logout --no-prompt";
        else if (de == "XFCE")
            return "gnome-session-quit --logout --no-prompt";
        else
            return "notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out";

    case SUSPEND:
        if (de == "XFCE")
            return "xfce4-session-logout --suspend";
        else
            return "systemctl suspend -i";

    case HIBERNATE:
        if (de == "XFCE")
            return "xfce4-session-logout --hibernate";
        else
            return "systemctl hibernate -i";

    case REBOOT:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --reboot";
        else if (de == "kde-plasma")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 1 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --reboot";
        else if (de == "XFCE")
            return "xfce4-session-logout --reboot";
        else
            return "notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot";

    case POWEROFF:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-session-quit --power-off --no-prompt";
        else if (de == "kde-plasma")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --power-off --no-prompt";
        else if (de == "XFCE")
            return "xfce4-session-logout --halt";
        else
            return "notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown";

    case NUMCOMMANDS:
        // NEVER REACHED;
        return "";
    }

    // NEVER REACHED;
    return "";
}
