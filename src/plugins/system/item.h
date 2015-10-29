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
#include <vector>
#include "abstractobjects.hpp"
using std::vector;

namespace System {
class Item final : public AlbertItem
{
    friend class Extension;
public:
    Item();
    ~Item();

    QString text() const override;
    QString subtext() const override;
    QIcon icon() const override;
    void activate() override;
    bool hasChildren() const override;
    vector<shared_ptr<AlbertItem>> children() override;

private:
    QString cmd_;
    mutable short usage_;
};
}
