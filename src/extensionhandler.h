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

#ifndef EXTENSIONHANDLER_H
#define EXTENSIONHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPluginLoader>
#include <QDebug>
#include <QMap>

#include "query.h"
#include "extensioninterface.h"

class ExtensionHandler : public QObject
{
    Q_OBJECT

public:
    void initialize();
    void finalize();

private:
    QSet<ExtensionInterface*> _extensions;
    QMap<QString, Query*> _recentQueries;
    QString _lastSearchTerm;


signals:
    void currentQueryChanged(Query *);

public slots:
    void startQuery(const QString &term);
    void setupSession();
    void teardownSession();
};

#endif // EXTENSIONHANDLER_H


/*

Hoe to load an extension











*/
