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
#include <QVariant>
#include <functional>
#include "core/query.h"
#include "player.h"
using std::function;
typedef std::shared_ptr<Core::Item> SharedItem;

namespace MPRIS {

class Command
{
public:
    /**
     * @brief Command   Constructs a DBus command to launch from albert with the given parameters.
     * @param label     An internal variable to query at a later point. Not needed for the DBus query.
     * @param title     The title of the StandardItem which will be created.
     * @param subtext   The subtext of the StandardItem which will be created.
     * @param method    The DBus method to invoke when this command is performed.
     * @param iconpath  The path to the icon which the StandardItem will get.
     */
    Command(const QString& label, const QString& title, const QString& subtext, const QString& method, QString iconpath);

    QString& getLabel();
    QString& getTitle();
    QString& getMethod();
    QString& getIconPath();

    /**
     * @brief applicableWhen    Configure this command to be only appicable under a certian (given) conditions.
     * @param path              The path to query the property from.
     * @param property          The name of the property. This property will be checked as condition.
     * @param expectedValue     The value of the property.
     * @param positivity        The result of the equality-check (queriedValue == expectedValue). Here you can negate the result.
     * @return                  Returns itself, but now configured for applicability-check
     */
    Command& applicableWhen(const char *path, const char* property, const QVariant expectedValue, bool positivity);

    /**
     * @brief produceStandardItem   Produces an instance of AlbertItem for this command to invoke on a given Player.
     * @return                      Returns a shared_ptr on this AlbertItem.
     */
    SharedItem produceAlbertItem(Player &) const;

    /**
     * @brief isApplicable  If configured, checks if the given property meets the expected criteria.
     * @return              True if not configured or match, false if the property is different than expected.
     */
    bool isApplicable(Player&) const;

private:
    QString label_, title_, subtext_, method_, iconpath_;
    bool applicableCheck_;
    QString path_;
    QString property_;
    QVariant expectedValue_;
    bool positivity_;
};

} // namespace MPRIS
