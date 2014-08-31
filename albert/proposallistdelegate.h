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
	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {


		if(option.state & QStyle::State_Selected){
			QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomRight());
			gradient.setColorAt(0, option.widget->palette().color(QPalette::Window).lighter(110)  );
			gradient.setColorAt(1, option.widget->palette().color(QPalette::Window));
			painter->fillRect(option.rect, gradient);
		}

		painter->drawPixmap(
					option.rect.x(),
					option.rect.y(),
					index.data(Qt::DecorationRole).value<QIcon>().pixmap(48,48));

		QFont font = option.font;
		font.setPixelSize(32);
		painter->setFont(font);
		painter->drawText(
					option.rect.x()+48,
					option.rect.y(),
					option.rect.width()-48,
					48,
					Qt::AlignTop|Qt::AlignLeft,
					index.data(Qt::DisplayRole).toString(),
					nullptr);

		font.setPixelSize(12);
		painter->setFont(font);
		painter->drawText(option.rect.x()+48,
						  option.rect.y()+36,
						  option.rect.width()-48,
						  option.rect.height()-32,
						  Qt::AlignTop|Qt::AlignLeft,
						  index.data(Qt::UserRole + 1).toString(),
						  nullptr);
	}

	QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const{
		return QSize(200, 48);
	}
};

#endif // PROPOSALLISTDELEGATE_H
