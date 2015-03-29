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
#include "stdint.h"
#include <QString>
#include <QAbstractListModel>
#include "extensioninterface.h"

/****************************************************************************///
struct QueryResult
{
	enum class Type :uint8_t {Interactive, Informational};

	QueryResult() = delete;
	explicit QueryResult(ExtensionInterface *ext)
		: _extension(ext) {}
    explicit QueryResult(ExtensionInterface *e, const QString &id, Type t, uint r)
        : rid(id), type(t), usage(r), _extension(e) {}
	~QueryResult(){}


    const QIcon  &icon     (const Query &q, Qt::KeyboardModifiers mods) const { return _extension->icon      (q, *this, mods); }
    void         action    (const Query &q, Qt::KeyboardModifiers mods) const {        _extension->action    (q, *this, mods); }
    QString      titleText (const Query &q, Qt::KeyboardModifiers mods) const { return _extension->titleText (q, *this, mods); }
    QString      infoText  (const Query &q, Qt::KeyboardModifiers mods) const { return _extension->infoText  (q, *this, mods); }
    QString      actionText(const Query &q, Qt::KeyboardModifiers mods) const { return _extension->actionText(q, *this, mods); }

	QString rid;
	Type    type;
    uint    usage;

private:
	ExtensionInterface * _extension;
};


/****************************************************************************///
class Query final : public QAbstractListModel
{
	Q_OBJECT

public:
    explicit Query(QString term) : _searchTerm(term), _dynamicSort(false) {}
	~Query(){}

	void addResults(const QList<QueryResult> &&results);
	void addResult(QueryResult &&result);

    int      rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    void     sort();

    void     setDynamicSearch(bool b){ _dynamicSort = b; }
    bool     dynamicSearch(){ return _dynamicSort; }

    const QString& searchTerm() {return _searchTerm;}

private:
	QList<QueryResult> _results;
	QString _searchTerm;
    bool    _dynamicSort;
};
