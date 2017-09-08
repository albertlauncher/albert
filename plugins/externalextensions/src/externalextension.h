// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include <QProcess>
#include <QMutex>
#include <map>
#include "core/queryhandler.h"

namespace ExternalExtensions {

class ExternalExtension final :
        public QObject,
        public Core::QueryHandler
{

public:

    ExternalExtension(const QString &path, const QString &id);
    ~ExternalExtension();

    /*
     * Implementation of extension interface
     */

    QStringList triggers() const override { return {trigger_}; }
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(Core::Query *query) const override;

    /*
     * Extension specific members
     */


    const QString &path() { return path_; }
    const QString &id() { return id_; }
    const QString &name() { return name_; }
    const QString &author() { return author_; }
    const QString &version() { return version_; }
    const QStringList &dependencies() { return dependencies_; }

private:

    QString runOperation(const QString &);

    QString path_;
    QString id_;
    QString name_;
    QString author_;
    QString version_;
    QStringList dependencies_;
    QString trigger_;
    mutable std::map<QString, QString> variables_;
    mutable QMutex processMutex_;
};

}
