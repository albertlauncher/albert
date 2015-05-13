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
#include "objects.h"
#include <QDesktopServices>
#include <QUrl>

class QString;
class QIcon;
class QMimeType;
class QMimeType;
namespace Files {
class FileIndex;



class File final : public AlbertObject {
    friend class FileIndex;
public:
    QString name() override;
    QString description() override;
    QStringList alises() override;
    QIcon icon() override;
    uint usage() override;
    QList<std::shared_ptr<Action>> actions() override;
    QString path();
    QMimeType mimetype();
    bool isDir();
    static void clearIconCache();
protected:
    int _id;
    QString _name;
    QString _path;
    QMimeType _mimetype;
    uint _usage;
    FileIndex *_fileIndex;
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
    void execute() const override { QDesktopServices::openUrl(QUrl("file:///"+_file.path()+"/"+_file.name())); }
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


typedef std::shared_ptr<File> SharedFilePtr;

}
