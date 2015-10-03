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

#include "application.h"
#include "desktopaction.h"
#include "albertapp.h"


/** ***************************************************************************/
Applications::DesktopAction::DesktopAction(Application *app, const QString &name, const QString &exec, const QIcon &icon, const bool term)
    : app_(app), name_(name), exec_(exec), icon_(icon), term_(term) {

}



/** ***************************************************************************/
QString Applications::DesktopAction::name() const {
    return name_;
}



/** ***************************************************************************/
QString Applications::DesktopAction::info() const {
    return exec_;
}



/** ***************************************************************************/
QIcon Applications::DesktopAction::icon() const {
    return icon_;
}



/** ***************************************************************************/
void Applications::DesktopAction::activate() {
    qApp->hideWidget();
    if(term_)
        return CommandAction(Application::terminal.arg(exec_)).activate();
    else
        return CommandAction(exec_).activate();
    app_->incUsage();
}
