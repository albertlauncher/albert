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

#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QWidget>
#include <QString>
#include <QIcon>
#include <QHash>
#include <QFileSystemWatcher>
#include <extensioninterface.h>
#include "ui_configwidget.h"
#include "settings.h"
#include "prefixsearch.h"

/****************************************************************************///
struct AppInfo {
	QString name;
	QString altName;
	QString iconName;
	QString exec;
	uint    usage;
};

/****************************************************************************///
class ConfigWidget final : public QWidget
{
	Q_OBJECT
public:
	explicit ConfigWidget(QWidget *parent = 0) : QWidget(parent) {ui.setupUi(this);}
	Ui::ConfigWidget ui;
};

/****************************************************************************///
class AppLauncher final : public QObject, public ExtensionInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
	Q_INTERFACES(ExtensionInterface)

public:
	explicit AppLauncher() : _search(nullptr) {}
    ~AppLauncher() {if (_search) delete _search;}

	void cleanApplications();
	void updateApplications(const QString &path);
	void removePath(const QString &path);
	void addPath(const QString &path);
	void serialize(const QString &path);
	void deserialize(const QString &path);

	/*
	 * ExtensionInterface
	 */
	QString     name() const override;
	QString     abstract() const override;
	void        initialize() override;
	void        finalize() override;
	void        setupSession() override;
	void        teardownSession() override;
	void        handleQuery(Query*) override;
	QString     text (const QueryResult&) const override;
	QString     subtext (const QueryResult&) const override;
	const QIcon &icon(const QueryResult&) override;
	void        action(const Query&, const QueryResult&) override;
	QWidget*    widget() override;

private:
	QHash<QString, AppInfo> _index;
	QHash<QString, QIcon>   _iconCache;
	QFileSystemWatcher      _watcher;
	AbstractSearch<AppInfo> *_search;

    static bool getAppInfo(const QString &path, AppInfo *appInfo);
    static QIcon getIcon(const QString &iconName);
	void restorePaths();
};

#endif // APPLAUNCHER_H




