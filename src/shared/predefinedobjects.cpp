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
#include <QProcess>
#include <QUrl>
#include <QString>
#include <QIcon>
#include <QVariant>
#include <memory>
#include <vector>
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
#include "albertapp.h"



/** ***************************************************************************/
/** *********************    A  C  T  I  O  N  S    ***************************/
/** ***************************************************************************/
class UrlAction final : public Action
{
public:
    UrlAction(QUrl url) : url_(url) {}
    UrlAction(QString url) : url_(QUrl(url)) {}
    void activate() override {
        QDesktopServices::openUrl(url_);
        //QProcess::startDetached(QString("kstart --activate chromium %1").arg(QUrl(_url).toString()));
        //QProcess::startDetached(QString("chromium %1").arg(QUrl(b._url).toString()));
    }
private:
    QUrl url_;
};



/** ***************************************************************************/
class CommandAction final : public Action
{
public:
    CommandAction(QString cmd) : cmd_(cmd) {}
    void activate() override { QProcess::startDetached(cmd_); }
private:
    QString cmd_;
};



/** ***************************************************************************/
class CopyToClipboardAction final : public Action
{
public:
    CopyToClipboardAction(QString text) : text_(text) {}
    void activate() override {
        QApplication::clipboard()->setText(text_);
    }
private:
    QString text_;
};



/** ***************************************************************************/
/** ************************    I  T  E  M  S    ******************************/
/** ***************************************************************************/



class StandardItem final : public A2Leaf
{
public:
    StandardItem(){}
    QString name() const override { return name_; }
    QString info() const override { return info_; }
    QIcon icon() const override { return icon_; }
    void activate() override {
        action_->activate();
        qApp->hideWidget();
    }

    void setName(QString name){name_ = name;}
    void setInfo(QString info){info_ = info;}
    void setIcon(QIcon icon){icon_ = icon;}
    void setAction(unique_ptr<Action> action){ action_ = std::move(action);}

private:
    QString name_;
    QString info_;
    QIcon icon_;
    unique_ptr<Action> action_;
};

