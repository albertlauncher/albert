// albert extension mpris - a mpris interface plugin for albert
// Copyright (C) 2016 Martin Buergmann
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
using std::vector;
#include "abstractitem.h"
#include "standardobjects.h"
#include "player.h"
#include <QDBusMessage>

namespace MPRIS {
class Item final : public AbstractItem
{
public:
    Item(Player& p, QString& subtext, QString& iconPath, QDBusMessage& msg, bool hideAfter);
    ~Item();

    QString id() const override { return id_; }
    QString text() const override { return text_; }
    QString subtext() const override { return subtext_; }
    QString iconPath() const override { return iconPath_; }
    vector<shared_ptr<AbstractAction>> actions() override;

private:
    Player& player_;
    QString id_;
    QString text_;
    QString subtext_;
    QString iconPath_;
    QDBusMessage message_;
    shared_ptr<AbstractAction> action_;
    bool hideAfter_;
};
}
