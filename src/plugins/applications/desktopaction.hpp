// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QProcess>
#include "abstractaction.h"
#include "albertapp.h"
#include "desktopentry.h"

namespace Applications {

class DesktopEntry::DesktopAction final : public AbstractAction
{
public:

    /** Consructor */
    DesktopAction(DesktopEntry *app, const QString &name, const QString &exec, const bool term=false)
        : app_(app), description_(name), exec_(exec), term_(term){}

    /** Returns the actions description */
    QString text(const QString &) const override { return description_; }

    /** Executes the action */
    void activate(ExecutionFlags *, const QString &) override {
        QProcess::startDetached((term_)? qApp->term().arg(exec_) : exec_);
        ++app_->usage_;
    }

    DesktopEntry * const app_;
    const QString description_;
    const QString exec_;
    const bool term_;
};

}
