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

namespace Applications {

QHash<QString, QIcon> Application::_iconCache;

/** ***************************************************************************/
Application::Application() : _usage(0) {}



/** ***************************************************************************/
QString Application::name(){
    return _name;
}



/** ***************************************************************************/
QString Application::description(){
    return _altName;
}



/** ***************************************************************************/
QStringList Application::alises(){
    return QStringList() << _name << _altName << _exec;
}



/** ***************************************************************************/
QIcon Application::icon(){
//    if (!_iconCache.contains(_mimetype.iconName())){
//        if (QIcon::hasThemeIcon(_mimetype.iconName()))
//            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme(_mimetype.iconName()));
//        else if(QIcon::hasThemeIcon(_mimetype.genericIconName()))
//            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme(_mimetype.genericIconName()));
//        else
//            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme("unknown"));
//    }
    return _icon;//_iconCache[_mimetype.iconName()];
}



/** ***************************************************************************/
uint Application::usage(){
    return _usage;
}



/** ***************************************************************************/
QList<std::shared_ptr<Action>> Application::actions(){
    return QList<std::shared_ptr<Action>>();// { std::make_shared<OpenFileAction>(*this), std::make_shared<RevealFileAction>(*this) };
    /*

    void Extension::action(const Item& ai, const Query &, Qt::KeyboardModifiers mods) const
    {
        QString cmd;
        if (mods == Qt::ControlModifier) cmd.prepend("gksu ");
        cmd.append(ai._exec);
        bool succ = QProcess::startDetached(cmd);
        if(!succ){
            QMessageBox msgBox(QMessageBox::Warning, "Error",
                               "Could not run \"" + cmd + "\"",
                               QMessageBox::Ok);
            msgBox.exec();
        }
    }

    QString Extension::actionText(const Item& ai, const Query &, Qt::KeyboardModifiers mods) const
    {
        if (mods == Qt::ControlModifier)
            return "Run " + ai._name + " as root";
        return "Run " + ai._name;
    }
    */
}
}






