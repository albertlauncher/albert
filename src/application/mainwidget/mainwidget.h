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
#include <QWidget>
#include <QLineEdit>
#include "proposallist.h"
#include "inputline.h"
#include "ui_mainwidget.h"
class QAbstractItemModel;

class MainWidget final : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = 0);
	~MainWidget();

    void show();
    void hide();
    void toggleVisibility();
    void setModel(QAbstractItemModel *);

    bool showCentered() const;
    void setShowCentered(bool b = true);

    const QString &theme() const;
    bool setTheme(const QString& theme);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool b = true);

    Ui::MainWidget ui;

private:
    void closeEvent(QCloseEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *) override;

    QString _theme;
    bool _showCentered;
    bool _hideOnFocusLoss;

    static const QString CFG_WND_POS;
    static const QString CFG_CENTERED;
    static const bool    DEF_CENTERED;
    static const QString CFG_THEME;
    static const QString DEF_THEME;
    static const QString CFG_HIDE_ON_FOCUS_LOSS;
    static const bool    DEF_HIDE_ON_FOCUS_LOSS;

signals:
    void widgetShown();
    void widgetHidden();
};
