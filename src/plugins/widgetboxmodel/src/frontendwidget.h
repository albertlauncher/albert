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
#include <memory>
#include "core_globals.h"
#include "core/frontend.h"

namespace WidgetBoxModel {

class FrontendWidget final : public QWidget
{
	Q_OBJECT

    class Private;

public:

    FrontendWidget();
    ~FrontendWidget();

    bool isVisible();
    void setVisible(bool visible) override;

    QString input();
    void setInput(const QString&);

    void setModel(QAbstractItemModel *);

    QWidget* widget(QWidget *parent = nullptr);

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

    uint8_t maxResults() const;
    void setMaxResults(uint8_t max);

    bool displayIcons() const;
    void setDisplayIcons(bool value);

    bool displayScrollbar() const;
    void setDisplayScrollbar(bool value);

    bool displayShadow() const;
    void setDisplayShadow(bool value);

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

    std::unique_ptr<Private> d;

signals:

    void widgetShown();
    void widgetHidden();
    void inputChanged(QString qry);
    void settingsWidgetRequested();
};

}
