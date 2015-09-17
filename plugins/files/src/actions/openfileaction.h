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
#include "abstractfileaction.h"
#include <QString>
#include <QIcon>
class Query;

namespace Files {
class OpenFileAction final : public AbtractFileAction
{
public:
    OpenFileAction(File *file) : AbtractFileAction(file) {}
    QString name(Query const *q) const override;
    QString description(Query const *q) const override;
    QIcon   icon() const override;
    void    activate(Query const *q) override;
    uint    usage() const override;
protected:
    static unsigned int usageCounter;
};
}
