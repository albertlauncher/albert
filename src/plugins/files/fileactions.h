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
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include "albertapp.h"
#include "file.h"

namespace Files {

/** ***************************************************************************/
class AbstractFileAction : public AlbertItem
{
public:
    AbstractFileAction(File *file) : file_(file) {}
    QString subtext() const override { return file_->path(); }

protected:
    File *file_;
};



/** ***************************************************************************/
class OpenFileAction final : public AbstractFileAction
{
public:
    OpenFileAction(File *file) : AbstractFileAction(file) {}

    QString text() const override {
        return "Open file in default application";
    }

    QString iconPath() const override {
        return file_->iconPath();
    }

    void activate() override {
        qApp->hideWidget();
        QDesktopServices::openUrl(QUrl::fromLocalFile(file_->path()));
        file_->incUsage();
    }
};



/** ***************************************************************************/
class RevealFileAction final : public AbstractFileAction
{
public:
    RevealFileAction(File *file) : AbstractFileAction(file) {}

    QString text() const override {
        return "Reveal file in default filebrowser";
    }

    QString iconPath() const override {
        return QString();// FIXME QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    }

    void activate() override {
        qApp->hideWidget();
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file_->path()).path()));
        file_->incUsage();
    }
};



/** ***************************************************************************/
class CopyFileAction final : public AbstractFileAction
{
public:
    CopyFileAction(File *file) : AbstractFileAction(file) {}

    QString text() const override {
        return "Copy file to clipboard";
    }

    QString iconPath() const override {
        return QString(); //  FIXME QIcon::fromTheme("edit-copy");
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
        newMimeData->setText(file_->path());

        // Copy file
        newMimeData->setUrls({QUrl::fromLocalFile(file_->path())});

        // Copy file (f*** you gnome)
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(file_->path()).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

        // Set the mimedata
        cb->setMimeData(newMimeData);

        file_->incUsage();
    }
};



/** ***************************************************************************/
class CopyPathAction final : public AbstractFileAction
{
public:
    CopyPathAction(File *file) : AbstractFileAction(file) {}

    QString text() const override {
        return "Copy path to clipboard";
    }

    QString iconPath() const override {
        return QString();// FIXME QIcon::fromTheme("edit-copy");
    }

    void activate() override {
        qApp->hideWidget();
        QApplication::clipboard()->setText(file_->path());
        file_->incUsage();
    }
};


}
