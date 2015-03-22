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

#ifndef QUERY_H
#define QUERY_H

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
	explicit QueryResult(ExtensionInterface *e, const QString &id, Type t, int8_t r)
		: rid(id), type(t), relevance(r), _extension(e) {}
	~QueryResult(){}

	QString text() const   { return _extension->text(*this); }
	QString subtext() const { return _extension->subtext(*this); }
	const QIcon& icon() const { return _extension->icon(*this); }
	void action(const Query &query) const {_extension->action(query, *this);}

	QString rid;
	Type    type;
	uint8_t relevance;

private:
	ExtensionInterface * _extension;
};


/****************************************************************************///
class Query final : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit Query(QString term) : _searchTerm(term) {}
	~Query(){}

	const QString& searchTerm() {return _searchTerm;}
//	const QList<QueryResult>& results() {return _results;}
	void addResults(const QList<QueryResult> &&results);
	void addResult(QueryResult &&result);

	int      rowCount(const QModelIndex & = QModelIndex()) const override;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

private:
	QList<QueryResult> _results;
	QString _searchTerm;

public slots:
	void run(const QModelIndex &);
};

#endif // QUERY_H

/*
 *  STUFF RELATED TO SORTING
 */
// std::stable_sort(_data.begin(), _data.end(), Service::Item::ATimeCompare());




