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
#include <QDesktopServices>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QFileInfo>
#include <QUrl>
#include "file.h"
#include "action.h"

namespace Files {

/** ***************************************************************************/
struct FileAction : public Core::Action
{
    FileAction(File *file);
    ~FileAction();
    File const * const file_;
};



/** ***************************************************************************/
struct OpenFileAction final : public FileAction
{
    OpenFileAction(File *file);
    QString text() const override;
    void activate() override;
};



/** ***************************************************************************/
struct RevealFileAction final : public FileAction
{
    RevealFileAction(File *file);
    QString text() const override;
    void activate() override;
};



/** ***************************************************************************/
struct TerminalFileAction final : public FileAction
{
    TerminalFileAction(File *file);
    QString text() const override;
    void activate() override;
};



/** ***************************************************************************/
struct CopyFileAction final : public FileAction
{
    CopyFileAction(File *file);
    QString text() const override;
    void activate() override;
};



/** ***************************************************************************/
struct CopyPathAction final : public FileAction
{
    CopyPathAction(File *file);
    QString text() const override;
    void activate() override;
};


}
