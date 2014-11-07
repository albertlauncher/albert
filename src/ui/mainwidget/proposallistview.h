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

#ifndef PROPOSALLISTVIEW_H
#define PROPOSALLISTVIEW_H

#include <QListWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QArrayData>


class ProposalListView: public QListView
{
	Q_OBJECT

	QAbstractItemDelegate *_selectedDelegate;
	QSet<int> _customDelegateRows;
	bool _subModeSelIsAction;
	bool _subModeDefIsAction;

	void modifyDelegate(Qt::KeyboardModifiers);

public:
	enum class SubTextMode{ None, Info, Action };

	explicit ProposalListView(QWidget *parent = 0);
	~ProposalListView();

	void setSubModeSel(SubTextMode d);
	void setSubModeDef(SubTextMode d);
	QSize sizeHint() const override;

protected:
	bool eventFilter(QObject*, QEvent *event) override;
	void currentChanged(const QModelIndex & current, const QModelIndex & previous) override;

signals:
	void completion(QString);

public slots:
	void reset() override;
};

#endif // PROPOSALLISTVIEW_H
