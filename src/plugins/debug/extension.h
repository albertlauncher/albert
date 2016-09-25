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
#include <QObject>
#include <QPointer>
#include "abstractextension.h"

namespace Debug {

class ConfigWidget;

class Extension final : public AbstractExtension
{
    Q_OBJECT
    Q_INTERFACES(AbstractExtension)
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "Debug"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Query query) override;
    bool runExclusive() const override {return true;}
    QStringList triggers() const override {return {trigger_};}

    uint count() const{return count_;}
    void setCount(const uint &count){count_ = count;}

    bool async() const{return async_;}
    void setAsync(bool async){async_ = async;}

    uint delay() const {return delay_;}
    void setDelay(const uint &delay) {delay_ = delay;}

    QString trigger() const {return trigger_;}
    void setTrigger(const QString &trigger){trigger_ = trigger;}

private:

    QPointer<ConfigWidget> widget_;

    uint delay_;
    uint count_;
    bool async_;
    QString trigger_;

};
}
