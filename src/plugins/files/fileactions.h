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
#include "abstractaction.h"

namespace Files {

/** ***************************************************************************/
struct AbstractFileAction : public AbstractAction
{
    AbstractFileAction(File *file);
    ~AbstractFileAction();
    File const * const file_;
};



/** ***************************************************************************/
struct File::OpenFileAction final : public AbstractFileAction
{
    OpenFileAction(File *file);
    QString text() const override;
    void activate(ExecutionFlags *) override;
};



/** ***************************************************************************/
struct File::RevealFileAction final : public AbstractFileAction
{
    RevealFileAction(File *file);
    QString text() const override;
    void activate(ExecutionFlags *) override;
};



/** ***************************************************************************/
struct File::CopyFileAction final : public AbstractFileAction
{
    CopyFileAction(File *file);
    QString text() const override;
    void activate(ExecutionFlags *) override;
};



/** ***************************************************************************/
struct File::CopyPathAction final : public AbstractFileAction
{
    CopyPathAction(File *file);
    QString text() const override;
    void activate(ExecutionFlags *) override;
};


}
