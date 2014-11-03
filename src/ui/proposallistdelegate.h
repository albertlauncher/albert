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

#ifndef PROPOSALLISTDELEGATE_H
#define PROPOSALLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>
#include <QDebug>
#include <QLinearGradient>

class ProposalListDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	int _role;

public:
	ProposalListDelegate(Qt::KeyboardModifiers mods);

protected:
	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
};

#endif // PROPOSALLISTDELEGATE_H
