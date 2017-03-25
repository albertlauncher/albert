// albert - a simple application launcher for linux
// Copyright (C) 2016-2017 Martin Buergmann
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

#include <QString>
#include "vmitem.h"

namespace VirtualBox {

class VM
{
public:
    VM(const QString vboxFileName);
    VMItem* produceItem() const;
    bool startsWith(QString other) const;
    const QString &uuid() const { return uuid_; }
    void probeState() const;

private:
    QString name_;
    QString uuid_;
    mutable QString state_;
};

} // namespace VirtualBox

