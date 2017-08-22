// albert extension mpris - a mpris interface plugin for albert
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

namespace MPRIS {

class Player
{
public:
    Player(const QString& name, const QString& busid, bool canRaise)
        : busId_(busid), name_(name), canRaise_(canRaise) {}

    const QString& name() const { return name_; }
    const QString& busId() const { return busId_; }
    bool canRaise() const { return canRaise_; }

private:

    QString busId_;
    QString name_;
    bool canRaise_;
};

} // namespace MPRIS
