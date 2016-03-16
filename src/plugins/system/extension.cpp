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
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "albertapp.h"


const QString System::Extension::EXT_NAME      = "system";
const QString System::Extension::CFG_POWEROFF  = "poweroff";
const QString System::Extension::DEF_POWEROFF  = "systemctl poweroff -i";
const QString System::Extension::CFG_REBOOT    = "reboot";
const QString System::Extension::DEF_REBOOT    = "systemctl reboot -i";
const QString System::Extension::CFG_SUSPEND   = "suspend";
const QString System::Extension::DEF_SUSPEND   = "systemctl suspend -i";
const QString System::Extension::CFG_HIBERNATE = "hibernate";
const QString System::Extension::DEF_HIBERNATE = "systemctl hibernate -i";
const QString System::Extension::CFG_LOCK      = "lock";
const QString System::Extension::DEF_LOCK      = "cinnamon-screensaver-command -l";


/** ***************************************************************************/
System::Extension::Extension() {
    qDebug() << "[Template] Initialize extension";

    // Load settings
    QSettings s;
    shared_ptr<StandardItem> sp;
    s.beginGroup(EXT_NAME);

    actions_.push_back({CFG_POWEROFF,
                        "Poweroff",
                        "Poweroff the machine.",
                        QIcon::hasThemeIcon("system-shutdown") ? QIcon::fromTheme("system-shutdown") : QIcon(":poweroff"),
                        s.value(CFG_POWEROFF, DEF_POWEROFF).toString()});

    actions_.push_back({CFG_REBOOT,
                        "Reboot",
                        "Reboot the machine.",
                        QIcon::hasThemeIcon("system-reboot") ? QIcon::fromTheme("system-reboot") : QIcon(":reboot"),
                        s.value(CFG_REBOOT, DEF_REBOOT).toString()});

    actions_.push_back({CFG_SUSPEND,
                        "Suspend",
                        "Suspend the machine.",
                        QIcon::hasThemeIcon("system-suspend") ? QIcon::fromTheme("system-suspend") : QIcon(":suspend"),
                        s.value(CFG_SUSPEND, DEF_SUSPEND).toString()});

    actions_.push_back({CFG_HIBERNATE,
                        "Hiberate",
                        "Hiberate the machine.",
                        QIcon::hasThemeIcon("system-suspend-hibernate") ? QIcon::fromTheme("system-suspend-hibernate") : QIcon(":hibernate"),
                        s.value(CFG_HIBERNATE, DEF_HIBERNATE).toString()});

    actions_.push_back({CFG_LOCK,
                        "Lock",
                        "Lock the session.",
                        QIcon::hasThemeIcon("system-lock") ? QIcon::fromTheme("system-lock") : QIcon(":lock"),
                        s.value(CFG_LOCK, DEF_LOCK).toString()});

    qDebug() << "[Template] Extension initialized";
}



/** ***************************************************************************/
System::Extension::~Extension() {
    qDebug() << "[Template] Finalize extension";

    // Save settings
    QSettings s;
    s.beginGroup(EXT_NAME);
    for (vector<ActionSpec>::iterator it = actions_.begin(); it != actions_.end(); ++it)
        s.setValue(it->id, it->cmd);

    qDebug() << "[Template] Extension finalized";
}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        // poweroff
        widget_->ui.lineEdit_poweroff->setText(command(CFG_POWEROFF));
        connect(widget_->ui.lineEdit_poweroff, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(CFG_POWEROFF, s);});

        // reboot
        widget_->ui.lineEdit_reboot->setText(command(CFG_REBOOT));
        connect(widget_->ui.lineEdit_reboot, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(CFG_REBOOT, s);});

        // suspend
        widget_->ui.lineEdit_suspend->setText(command(CFG_SUSPEND));
        connect(widget_->ui.lineEdit_suspend, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(CFG_SUSPEND, s);});

        // hibernate
        widget_->ui.lineEdit_hibernate->setText(command(CFG_HIBERNATE));
        connect(widget_->ui.lineEdit_hibernate, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(CFG_HIBERNATE, s);});

        // lock
        widget_->ui.lineEdit_lock->setText(command(CFG_LOCK));
        connect(widget_->ui.lineEdit_lock, &QLineEdit::textEdited,
                [this](const QString &s){setCommand(CFG_LOCK, s);});

    }
    return widget_;
}



/** ***************************************************************************/
void System::Extension::handleQuery(shared_ptr<Query> query) {

    for (vector<ActionSpec>::iterator it = actions_.begin(); it != actions_.end(); ++it)
        if (it->name.toLower().startsWith(query->searchTerm()))
            query->addMatch(std::make_shared<StandardItem>(it->name,
                                                           it->desc,
                                                           it->icon,
                                                           [=](){
                qApp->hideWidget();
                QProcess::startDetached(it->cmd);
            }));
}



/** ***************************************************************************/
QString System::Extension::command(const QString& id){
    vector<ActionSpec>::iterator it = std::find_if(actions_.begin(), actions_.end(),
                 [&id](const ActionSpec& as){
                     return id==as.id;
                 });
    return (it != actions_.end()) ? it->cmd : "";
}



/** ***************************************************************************/
void System::Extension::setCommand(const QString& id, const QString& cmd){
    vector<ActionSpec>::iterator it = std::find_if(actions_.begin(), actions_.end(),
                 [&id](const ActionSpec& as){
                     return id==as.id;
                 });
    if (it != actions_.end())
        it->cmd = cmd;
}
