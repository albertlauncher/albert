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
#include <QIcon>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QStyle>
#include "albertapp.h"
#include "interfaces/baseobjects.h"
#include "file.h"

namespace Files {

/** ***************************************************************************/
class AbstractFileAction : public A2Leaf
{
public:
    AbstractFileAction(File *file) : _file(file) {}
    QString info() const override { return _file->path(); }

protected:
    File *_file;
};



/** ***************************************************************************/
class OpenFileAction final : public AbstractFileAction
{
public:
    OpenFileAction(File *file) : AbstractFileAction(file) {}

    QString name() const override {
        return "Open file in default application";
    }

    QIcon icon() const override {
        return _file->icon();
    }

    void activate() override {
        qApp->hideWidget();
        UrlAction(QUrl::fromLocalFile(_file->path())).activate();
        _file->incUsage();
    }
};



/** ***************************************************************************/
class RevealFileAction final : public AbstractFileAction
{
public:
    RevealFileAction(File *file) : AbstractFileAction(file) {}

    QString name() const override {
        return "Reveal file in default filebrowser";
    }

    QIcon icon() const override {
        return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    }

    void activate() override {
        qApp->hideWidget();
        UrlAction(QUrl::fromLocalFile(QFileInfo(_file->path()).path())).activate();
        _file->incUsage();
    }
};



/** ***************************************************************************/
class CopyFileAction final : public AbstractFileAction
{
public:
    CopyFileAction(File *file) : AbstractFileAction(file) {}

    QString name() const override {
        return "Copy file to clipboard";
    }

    QIcon icon() const override {
        return QIcon::fromTheme("edit-copy");
    }

    void activate() override {
        qApp->hideWidget();

        //  Get clipboard
        QClipboard *cb = QApplication::clipboard();

        // Ownership of the new data is transferred to the clipboard.
        QMimeData* newMimeData = new QMimeData();

        // Copy old mimedata
        const QMimeData* oldMimeData = cb->mimeData();
        for ( const QString &f : oldMimeData->formats())
            newMimeData->setData(f, oldMimeData->data(f));

        // Copy path of file
        newMimeData->setText(_file->path());

        // Copy file
        newMimeData->setUrls({QUrl::fromLocalFile(_file->path())});

        // Copy file (f*** you gnome)
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(_file->path()).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

        // Set the mimedata
        cb->setMimeData(newMimeData);

        _file->incUsage();
    }
};



/** ***************************************************************************/
class CopyPathAction final : public AbstractFileAction
{
public:
    CopyPathAction(File *file) : AbstractFileAction(file) {}

    QString name() const override {
        return "Copy path to clipboard";
    }

    QIcon icon() const override {
        return QIcon::fromTheme("edit-copy");
    }

    void activate() override {
        qApp->hideWidget();
        QApplication::clipboard()->setText(_file->path());
        _file->incUsage();
    }
};


}
