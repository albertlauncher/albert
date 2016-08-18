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
#include <vector>
using std::vector;
#include "iindexable.h"
#include "abstractitem.h"

namespace Applications{

class DesktopEntry final : public AbstractItem, public IIndexable
{
    class DesktopAction;

public:

    DesktopEntry();
    DesktopEntry(const QString &path);

    /*
     * Implementation of Item interface
     */

    QString id() const override { return path_; }
    QString text() const override { return name_; }
    QString subtext() const override { return altName(); }
    QString iconPath() const override { return iconPath_; }
    vector<IIndexable::WeightedKeyword> indexKeywords() const override;
    void activate(ExecutionFlags *) override;
    vector<SharedAction> actions() override { return actions_; }

    /*
     * Item specific members
     */

    /** Return the path of the desktop entry */
    const QString& path() const { return path_; }

    /** Sets the path of the desktop entry */
    void setPath(const QString& path) { path_ = path; }

    /** Parse the contents of the desktop entry */
    bool parseDesktopEntry();

    /** Return the name of the application */
    const QString& name() const { return name_; }

    /** Return the alternative of the application */
    const QString& altName() const { return (altName_.isNull()) ? exec_ : altName_; }

    /** Return the command to run the application */
    const QString& exec() const { return exec_; }

    /** Indicates if this is a terminal application */
    bool term() const { return term_; }

    /** Serialize the desktop entry */
    void serialize(QDataStream &out);

    /** Deserialize the desktop entry */
    void deserialize(QDataStream &in);

private:
    /** Translate escape sequences in a string */
    static QString escapeString(const QString &unescaped);
    static QString quoteString(const QString &unquoted);
    static QStringList execValueEscape(const QString &value);

    QString path_;

    // Updated by parseDesktopEntry()
    QString name_;
    QString altName_;
    QString iconPath_;
    QString exec_;
    bool term_;
    vector<SharedAction> actions_;

    /** The list of supported graphical sudo applications */
    static QStringList supportedGraphicalSudo;
};
}
