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
#include <QObject>
#include "extension.h"
#include "queryhandler.h"

namespace Files {

class FilesPrivate;
class ConfigWidget;

class Extension final :
        public QObject,
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    /*
     * Implementation of extension interface
     */

    QString name() const override { return "Files"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) override;

    /*
     * Extension specific members
     */

    void addDir(const QString &dirPath);
    void removeDir(const QString &dirPath);
    void restorePaths();

    // Properties
    bool indexAudio();
    void setIndexAudio(bool b = true);

    bool indexVideo();
    void setIndexVideo(bool b = true);

    bool indexImage();
    void setIndexImage(bool b = true);

    bool indexDocs();
    void setIndexDocs(bool b = true);

    bool indexDirs();
    void setIndexDirs(bool b = true);

    bool indexHidden();
    void setIndexHidden(bool b = true);

    bool followSymlinks();
    void setFollowSymlinks(bool b = true);

    uint scanInterval();
    void setScanInterval(uint minutes);

    bool fuzzy();
    void setFuzzy(bool b = true);

    void updateIndex();

private:

    FilesPrivate *d;

signals:

    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);

};
}
