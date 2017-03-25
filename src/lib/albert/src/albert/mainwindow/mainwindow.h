// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include "history.h"
#include "ui_mainwindow.h"
class QAbstractItemModel;

class MainWindow final : public QWidget
{
	Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);

    void setVisible(bool visible) override;
    void toggleVisibility();

    void setInput(const QString&);

    bool showCentered() const;
    void setShowCentered(bool b = true);

    const QString &theme() const;
    bool setTheme(const QString& theme);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool b = true);

    bool hideOnClose() const;
    void setHideOnClose(bool b = true);

    bool clearOnHide() const;
    void setClearOnHide(bool b = true);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    uint8_t maxProposals() const;
    void setMaxProposals(uint8_t max);

    bool displayIcons() const;
    void setDisplayIcons(bool value);

    bool displayScrollbar() const;
    void setDisplayScrollbar(bool value);

    bool displayShadow() const;
    void setDisplayShadow(bool value);

    void setModel(QAbstractItemModel *);

    bool actionsAreShown() const;
    void setShowActions(bool showActions);
    void showActions() { setShowActions(true); }
    void hideActions() { setShowActions(false); }

protected:

    void closeEvent(QCloseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
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

    /** Indicates that the app should be hidden on close event */
    bool hideOnClose_;

    /** The offset from cursor to topleft. Used when the window is dagged */
    QPoint clickOffset_;

    /** Indcates the state that the app is in */
    bool actionsShown_;

    /** Indcates that a shadow should be drawn */
    bool displayShadow_;

    /** Indcates that the inputline should be cleared on hide */
    bool clearOnHide_;

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

signals:
    void widgetShown();
    void widgetHidden();
    void inputChanged(QString qry);
    void settingsWidgetRequested();
};
