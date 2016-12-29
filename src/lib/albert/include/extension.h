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
#include <QString>
#include <QWidget>
#include "core_globals.h"

namespace Core {

/**
 * @brief The extension interface
 */
class EXPORT_CORE Extension
{
public:

    Extension(const QString &id) : id(id) {}
    virtual ~Extension() {}

    /**
     * @brief An immutable application-wide unique identifier
     */
    const QString id;

    /**
     * @brief A human readable name of the plugin
     * @return The human readable name
     */
    virtual QString name() const = 0;

    /**
     * @brief The settings widget factory
     * This has to return the widget that is accessible to the user from the
     * albert settings plugin tab. If the return value is a nullptr there will
     * be no settings widget available in the settings.
     * @return The settings widget
     */
    virtual QWidget* widget(QWidget *parent = nullptr) = 0;

};

}
