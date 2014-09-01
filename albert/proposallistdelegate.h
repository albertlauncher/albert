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

public:
	ProposalListDelegate();
protected:
	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	inline QSize sizeHint ( const QStyleOptionViewItem&, const QModelIndex& ) const { return QSize(200, 48); }

};

#endif // PROPOSALLISTDELEGATE_H
