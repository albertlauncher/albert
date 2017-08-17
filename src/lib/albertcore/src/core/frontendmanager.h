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
#include <memory>

namespace Core {

class Frontend;
class PluginSpec;
class FrontendManagerPrivate;

class FrontendManager : public QObject
{
    Q_OBJECT

public:

    FrontendManager(QStringList pluginroots);
    ~FrontendManager();

    const std::vector<std::unique_ptr<PluginSpec> > &frontendSpecs() const;

    Frontend *currentFrontend();
    bool setCurrentFrontend(QString id);

    static FrontendManager *instance;

private:

    std::unique_ptr<FrontendManagerPrivate> d;

signals:

    void frontendLoaded(Frontend*);
    void frontendAboutToUnload(Frontend*);

};

}

