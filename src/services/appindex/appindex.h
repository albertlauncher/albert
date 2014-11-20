// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#ifndef APPINDEX_H
#define APPINDEX_H

#include "abstractindex.h"
#include <QFileSystemWatcher>

class AppIndex final : public Service, public AbstractIndex
{
public:
	class Item;

	AppIndex();
	~AppIndex();

	QWidget* widget() override;
	inline QString moduleName() override {return "AppLauncher";}

	void initialize() override;

	QStringList paths() const;
	bool addPath(const QString &s);
	bool removePath(const QString &s);
	void restorePaths();

	void saveSettings(QSettings &s) const override;
	void loadSettings(QSettings &s) override;
	void serilizeData(QDataStream &out) const override;
	void deserilizeData(QDataStream &in) override;

	void query(const QString &req, QVector<Service::Item*> *res) const override;
	void queryFallback(const QString&, QVector<Service::Item*>*) const override;

private:
	QFileSystemWatcher    _watcher;
	static QIcon getIcon(QString iconName);

public slots:
	void buildIndex() override;
};

#endif // APPINDEX_H
