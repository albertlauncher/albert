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

#pragma once
#include <QVariant>
#include "interfaces/baseobjects.h"

namespace Applications {
class Application;

class DesktopAction final : public A2Leaf
{
public:
    DesktopAction(Application *app, const QString &name, const QString &exec, const QIcon &icon, const bool term=false);

    QString name() const override;
    QString info() const override;
    QIcon icon() const override;
    void activate() override;

private:
    Application * const app_;
    const QString name_;
    const QString exec_;
    const QIcon icon_;
    const bool term_;
};

}
