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
#include <QDBusMessage>
#include <QVariant>
#include <vector>
#include "util/standarditem.h"
#include "player.h"
using std::vector;
using std::shared_ptr;
using Core::Action;

namespace MPRIS {

class Item final : public Core::Item
{
public:

    Item(Player &p, const QString& title, const QString& subtext, const QString& iconPath, const QDBusMessage& msg);
    ~Item();

    QString id() const override { return id_; }
    QString text() const override { return text_; }
    QString subtext() const override { return subtext_; }
    QString iconPath() const override { return iconPath_; }
    vector<shared_ptr<Action>> actions() override;

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString iconPath_;
    QDBusMessage message_;
    vector<shared_ptr<Action>> actions_;

};

}
