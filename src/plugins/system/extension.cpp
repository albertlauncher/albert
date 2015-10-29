// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <functional>
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "albertapp.h"


struct CommandAction {
    CommandAction(QString _cmd) : cmd(_cmd) {}
    QString cmd;
    void operator()(){
        qApp->hideWidget();
        QProcess::startDetached(cmd);
    }
};

/** ***************************************************************************/
System::Extension::Extension()
    : IExtension("org.albert.extension.system",
                 tr("System"),
                 tr("Control your system/session via albert")) {
    qDebug() << "Initialize extension:" << id;

    titles_ = {
        "Poweroff",
        "Reboot",
        "Suspend",
        "Hiberate",
        "Logout",
        "Lock"
    };
    descriptions_ = {
        "Poweroff the machine.",
        "Reboot the machine.",
        "Suspend the machine.",
        "Hiberate the machine.",
        "Logout the session.",
        "Lock the session."
    };
    iconpaths_ = {
        ":poweroff",
        ":reboot",
        ":suspend",
        ":hibernate",
        ":logout",
        ":lock"
    };
    defaults_ = {
        "dbus-send --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.RequestShutdown",
        "dbus-send --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.RequestReboot",
        "systemctl suspend -i",
        "systemctl hibernate -i",
        "dbus-send --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Logout",
        "cinnamon-screensaver-command -l"
    };

    // Load settings
    QSettings s;
    s.beginGroup(id);

    for (int i = 0; i <  static_cast<int>(Actions::NUM_ACTIONS); ++i) {
        index_.push_back(std::make_shared<StandardItem>(
                             titles_[i],
                             descriptions_[i],
                             QIcon(iconpaths_[i]),
                             CommandAction(s.value(titles_[i].toLower(),
                                                   defaults_[i]).toString())));
    }

    qDebug() << "Initialization done:" << id;
}



/** ***************************************************************************/
System::Extension::~Extension() {
    qDebug() << "Finalize extension:" << id;

    // Save settings
    QSettings s;
    s.beginGroup(id);
    for (vector<shared_ptr<AlbertItem>>::iterator it = index_.begin(); it != index_.end(); ++it)
        s.setValue(std::static_pointer_cast<StandardItem>(*it)->text().toLower(),
                   std::static_pointer_cast<StandardItem>(*it)->action().target<CommandAction>()->cmd);

    qDebug() << "Finalization done:" << id;
}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // poweroff
        widget_->ui.lineEdit_poweroff->setText(command(Actions::POWEROFF));
        connect(widget_->ui.lineEdit_poweroff, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::POWEROFF, s);});

        // reboot
        widget_->ui.lineEdit_reboot->setText(command(Actions::REBOOT));
        connect(widget_->ui.lineEdit_reboot, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::REBOOT, s);});

        // suspend
        widget_->ui.lineEdit_suspend->setText(command(Actions::SUSPEND));
        connect(widget_->ui.lineEdit_suspend, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::SUSPEND, s);});

        // hibernate
        widget_->ui.lineEdit_hibernate->setText(command(Actions::HIBERNATE));
        connect(widget_->ui.lineEdit_hibernate, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::HIBERNATE, s);});

        // logout
        widget_->ui.lineEdit_logout->setText(command(Actions::LOGOUT));
        connect(widget_->ui.lineEdit_logout, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::LOGOUT, s);});

        // lock
        widget_->ui.lineEdit_lock->setText(command(Actions::LOCK));
        connect(widget_->ui.lineEdit_lock, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(Actions::LOCK, s);});

        // Restore
        connect(widget_->ui.pushButton_restore, &QPushButton::clicked,
                this, &Extension::restoreCommands);
    }
    return widget_;
}



/** ***************************************************************************/
vector<shared_ptr<AlbertItem> > System::Extension::staticItems() const {
    return index_;
}



/** ***************************************************************************/
QString System::Extension::command(Actions action) {
    return std::static_pointer_cast<StandardItem>(index_[static_cast<int>(action)])
            ->action().target<CommandAction>()->cmd;
}



/** ***************************************************************************/
void System::Extension::setCommand(Actions action, const QString& cmd){
    std::static_pointer_cast<StandardItem>(index_[static_cast<int>(action)])
            ->action().target<CommandAction>()->cmd = cmd;
}



/** ***************************************************************************/
void System::Extension::restoreCommands() {
    for (int i = 0; i <  static_cast<int>(Actions::NUM_ACTIONS); ++i)
        setCommand(static_cast<Actions>(i), defaults_[i]);
}
