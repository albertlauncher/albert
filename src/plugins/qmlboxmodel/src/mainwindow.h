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
#include <QQuickView>
#include <QPoint>
#include <QIdentityProxyModel>
#include "core/history.h"

class MainWindow final : public QQuickView
{
    Q_OBJECT

public:

    MainWindow(QWindow *parent = 0);
    ~MainWindow();

    void setVisible(bool visible = true);

    QString input();
    void setInput(const QString&);

    void setModel(QAbstractItemModel* model);
    void setSource(const QUrl & url);

    QStringList availableProperties();
    QVariant property(const char *name) const;
    void setProperty(const char *attribute, const QVariant &value);

    QStringList availablePresets();
    void setPreset(const QString& name);

    bool showCentered() const;
    void setShowCentered(bool showCentered);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool hideOnFocusLoss);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    static const QString CFG_CENTERED;
    static const bool    DEF_CENTERED;
    static const QString CFG_HIDEONFOCUSLOSS;
    static const bool    DEF_HIDEONFOCUSLOSS;
    static const QString CFG_ALWAYS_ON_TOP;
    static const bool    DEF_ALWAYS_ON_TOP;
    static const QString CFG_STYLEPATH;
    static const QUrl    DEF_STYLEPATH;
    static const QString CFG_WND_POS;

protected:

    bool event(QEvent *event) override;

    bool showCentered_;
    bool hideOnFocusLoss_;
    History history_;
    QIdentityProxyModel model_;

signals:

    void inputChanged(QString input);
    void settingsWidgetRequested();

};
