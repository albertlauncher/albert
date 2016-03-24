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
#include <QStringListModel>
#include "proposallist.h"
#include "settingsbutton.h"
#include "history.hpp"
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

    void setModel(QAbstractItemModel *);

    bool actionsAreShown() const;
    void setShowActions(bool showActions);
    void showActions() { setShowActions(true); }
    void hideActions() { setShowActions(false); }

protected:

    void closeEvent(QCloseEvent * event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject*, QEvent *event) override;

private:

    /** The name of the current theme */
    QString theme_;

    /** Indicates that the app should be shown centered */
    bool showCentered_;

    /** Indicates that the app should be hidden on focus loss */
    bool hideOnFocusLoss_;

    /** The offset from cursor to topleft. Used when the window is dagged */
    QPoint clickOffset_;

    /** Indcates the state that the app is in */
    bool actionsShown_;

    /** The model of the action list view */
    QStringListModel *actionsListModel_;

    /** The button to open the settings dialog */
    SettingsButton *settingsButton_;

    /** The input history */
    History *history_;

    /** The modifier used to navigate directly in the history */
    Qt::KeyboardModifier historyMoveMod_;

    /** The form of the main app */
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
    static const char*   CFG_MAX_PROPOSALS;
    static const uint8_t DEF_MAX_PROPOSALS;
    static const char*   SETTINGS_SHORTCUT;

signals:
    void widgetShown();
    void widgetHidden();
    void startQuery(QString qry);
    void settingsWindowRequested();
};
