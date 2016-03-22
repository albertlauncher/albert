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
#include "ui_mainwindow.h"
class QAbstractItemModel;

class MainWindow final : public QWidget
{
	Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

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

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    uint8_t maxProposals() const;
    void setMaxProposals(uint8_t max);


protected:

    void closeEvent(QCloseEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent * event) override;
    bool event(QEvent *event) override;

private:

    QString theme_;
    bool showCentered_;
    bool hideOnFocusLoss_;
    QPoint clickOffset_;
    Ui::MainWindow ui;

    static const QString CFG_WND_POS;
    static const QString CFG_CENTERED;
    static const bool    DEF_CENTERED;
    static const QString CFG_THEME;
    static const QString DEF_THEME;
    static const QString CFG_HIDE_ON_FOCUS_LOSS;
    static const bool    DEF_HIDE_ON_FOCUS_LOSS;
    static const QString CFG_ALWAYS_ON_TOP;
    static const bool    DEF_ALWAYS_ON_TOP;

signals:
    void widgetShown();
    void widgetHidden();
    void startQuery(QString qry);
    void settingsWindowRequested();
    void activated(const QModelIndex &index);
};
