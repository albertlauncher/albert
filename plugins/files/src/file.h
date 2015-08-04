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
#include <QDesktopServices>
#include <QUrl>
#include <string>
using std::string;
#include "objects.h"

class QString;
class QIcon;
class QMimeType;

namespace Files {

class File final : public AlbertObject {

    friend class ScanWorker;

public:
    explicit File(const QString &path, QMimeType mimetype);

    QString name() override;
    QString description() override;
    QStringList alises() override;
    QIcon icon() override;
    uint usage() override;
    QList<std::shared_ptr<Action>> actions() override;

    QString path();
    QString absolutePath();
    QMimeType mimetype();
    bool isDir();
    static void clearIconCache();

private:
    string _path;
    QMimeType _mimetype;
    uint _usage;
    static QHash<QString, QIcon> _iconCache;
};










class OpenFileAction final : public Action {
public:
    OpenFileAction(File& file) : _file(file) {}
    ~OpenFileAction() {}

    QString name() override { return "Open file"; }
    QString description() override { return QString("Open %1 in default application.").arg(_file.name()); }
    QStringList alises() override { return QStringList(); }
    QIcon icon() override { return QIcon(); }
    AlbertObject* object() const override { return &_file; }
    void execute() const override { QDesktopServices::openUrl(QUrl("file://"+_file.absolutePath())); }
private:
    File& _file;
};



class RevealFileAction final : public Action {
public:
    RevealFileAction(File& file) : _file(file) {}
    ~RevealFileAction() {}

    QString name() override { return "Reveal file in filebrowser"; }
    QString description() override { return QString("Open %1's folder in default file browser.").arg(_file.name()); }
    QStringList alises() override { return QStringList(); }
    QIcon icon() override { return QIcon(); }
    AlbertObject* object() const override { return &_file; }
    void execute() const override { QDesktopServices::openUrl(QUrl("file:///"+_file.path()+"/")); }
private:
    File& _file;
};


typedef QSharedPointer<File> SharedFile;

}
