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

#include <QDebug>
#include <QProcess>
#include <QSettings>
#include "configwidget.h"
#include "main.h"
#include "query.h"
#include "standarditem.h"
#include "standardaction.h"
#include "xdgiconlookup.h"


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
System::Extension::Extension() : Core::Extension("org.albert.extension.system") {

    // Load settings
    QSettings s(qApp->applicationName());
    s.beginGroup(id);
    for (int i = 0; i < NUMCOMMANDS; ++i) {
        iconPaths.push_back(XdgIconLookup::instance()->themeIconPath(iconNames[i]));
        commands.push_back(s.value(configNames[i], defaultCommand(static_cast<SupportedCommands>(i))).toString());
    }
    s.endGroup();
}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // Initialize the content and connect the signals

        widget_->ui.lineEdit_lock->setText(commands[LOCK]);
        connect(widget_->ui.lineEdit_lock, &QLineEdit::textEdited, [this](const QString &s){
            commands[LOCK]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[LOCK]), s);
        });

        widget_->ui.lineEdit_logout->setText(commands[LOGOUT]);
        connect(widget_->ui.lineEdit_logout, &QLineEdit::textEdited, [this](const QString &s){
            commands[LOGOUT]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[LOGOUT]), s);
        });

        widget_->ui.lineEdit_suspend->setText(commands[SUSPEND]);
        connect(widget_->ui.lineEdit_suspend, &QLineEdit::textEdited, [this](const QString &s){
            commands[SUSPEND]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[SUSPEND]), s);
        });

        widget_->ui.lineEdit_hibernate->setText(commands[HIBERNATE]);
        connect(widget_->ui.lineEdit_hibernate, &QLineEdit::textEdited, [this](const QString &s){
            commands[HIBERNATE]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[HIBERNATE]), s);
        });

        widget_->ui.lineEdit_reboot->setText(commands[REBOOT]);
        connect(widget_->ui.lineEdit_reboot, &QLineEdit::textEdited, [this](const QString &s){
            commands[REBOOT]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[REBOOT]), s);
        });

        widget_->ui.lineEdit_shutdown->setText(commands[POWEROFF]);
        connect(widget_->ui.lineEdit_shutdown, &QLineEdit::textEdited, [this](const QString &s){
            commands[POWEROFF]= s;
            QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(id, configNames[POWEROFF]), s);
        });
    }
    return widget_;
}



/** ***************************************************************************/
void System::Extension::handleQuery(Core::Query * query) {
   for (int i = 0; i < NUMCOMMANDS; ++i) {
        if (configNames[i].startsWith(query->searchTerm().toLower())) {

            std::shared_ptr<Core::StandardItem> item = std::make_shared<Core::StandardItem>(configNames[i]);
            item->setText(itemTitles[i]);
            item->setSubtext(itemDescriptions[i]);
            item->setIconPath(iconPaths[i]);

           QString cmd = commands[i];
            std::shared_ptr<Core::StandardAction> action = std::make_shared<Core::StandardAction>();
            action->setText(itemDescriptions[i]);
            action->setAction([=](){
                QProcess::startDetached(cmd);
            });

            item->setActions({action});

            query->addMatch(item);
       }
   }
}



/** ***************************************************************************/
QString System::Extension::defaultCommand(SupportedCommands command){

    QString de = getenv("XDG_CURRENT_DESKTOP");

    switch (command) {
    case LOCK:
        if (de == "Unity" || de == "Pantheon" || de == "Gnome")
            return "gnome-screensaver-command --lock";
        else if (de == "XFCE")
            return "xflock4";
        if (de == "X-Cinnamon")
            return "cinnamon-screensaver-command --lock";
        else if (de == "MATE")
            return "mate-screensaver-command --lock";
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
        else if (de == "kde-plasma")
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
            return "gnome-session-quit --power-off --no-prompt";
        else if (de == "kde-plasma")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --power-off --no-prompt";
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
