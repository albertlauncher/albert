// albert - a simple application launcher for linux
// Copyright (C) 2016 Martin Buergmann
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
#include <QObject>
#include <QPointer>
#include <QFileSystemWatcher>
#include <QSqlDatabase>
#include <QList>
#include <QStringList>
#include <QMutex>
#include "abstractextension.h"
#include "standardobjects.h"
#include "offlineindex.h"

namespace FirefoxBookmarks {

class ConfigWidget;

class Extension final : public AbstractExtension
{
    Q_OBJECT
    Q_INTERFACES(AbstractExtension)
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

    class Indexer;

public:
    Extension();
    ~Extension();

    /*
     * Implementation of extension interface
     */

    QString name() const override { return name_; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(Query query) override;

    /*
     * Extension specific members
     */
    const static QVariant nullVariant;

public slots:
    void reloadConfig(QString);
    void scanProfiles(QString profilesIni);

private:
    // Gerneral stuff
    QPointer<ConfigWidget> widget_;
    const char* const name_ = "Firefox Bookmarks";
    bool enabled_;

    // places stuff
    QFileSystemWatcher placesWatcher_;
    QSqlDatabase base_;

    // profiles stuff
    QFileSystemWatcher profilesWatcher_;
    QString profilesIniPath_;
    QString currentProfile_;
    QStringList profiles_;

    // inedx stuff
    OfflineIndex offlineIndex_;
    QMutex indexAccess_;
    vector<shared_ptr<StandardIndexItem>> index_;
};
}
