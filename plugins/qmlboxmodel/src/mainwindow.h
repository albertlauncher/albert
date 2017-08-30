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
#include <QSettings>
#include <QPoint>
#include <QFileSystemWatcher>
#include <QIdentityProxyModel>
#include "core/history.h"

namespace QmlBoxModel {

struct QmlStyleSpec {
    QString name;
    QString version;
    QString author;
    QString mainComponent;
};

class FrontendPlugin;

class MainWindow final : public QQuickView
{
    Q_OBJECT

public:

    MainWindow(FrontendPlugin *plugin, QWindow *parent = 0);
    ~MainWindow();

    QString input();
    void setInput(const QString&);
    void setModel(QAbstractItemModel* model);

    const std::vector<QmlStyleSpec> &availableStyles() const;
    QStringList settableProperties();

    QVariant property(const char *name) const;
    void setProperty(const char *attribute, const QVariant &value);

    QStringList availableThemes();
    void setTheme(const QString& name);

    void setSource(const QUrl & url);

    bool showCentered() const;
    void setShowCentered(bool showCentered);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool hideOnFocusLoss);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    bool hideOnClose() const;
    void setHideOnClose(bool b = true);

protected:

    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    bool showCentered_;
    bool hideOnFocusLoss_;
    bool hideOnClose_;
    Core::History history_;
    QIdentityProxyModel model_;
    std::vector<QmlStyleSpec> styles_;
    QFileSystemWatcher watcher_;
    FrontendPlugin *plugin_;

signals:

    void inputChanged(QString input);
    void settingsWidgetRequested();

};

}
