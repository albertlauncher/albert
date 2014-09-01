#include "proposallistdelegate.h"
#include <QMimeType>
#include <QMimeDatabase>

ProposalListDelegate::ProposalListDelegate()
{
}

void ProposalListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{

	// Draw selection
	if(option.state & QStyle::State_Selected){
		QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomRight());
		gradient.setColorAt(0, option.widget->palette().color(QPalette::Window).lighter(120)  );
		gradient.setColorAt(1, option.widget->palette().color(QPalette::Window));
		painter->fillRect(option.rect, gradient);
	}

	// Draw icon
	QString info = index.data(Qt::UserRole).toString();
	painter->drawPixmap(option.rect.x(),
						option.rect.y(),
						QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(info).iconName()).pixmap(48,48));

	// Draw name
	QFont font = option.font;
	font.setPixelSize(32);
	painter->setFont(font);
	painter->drawText(option.rect.x()+48,
					  option.rect.y(),
					  option.rect.width()-48,
					  48,
					  Qt::AlignTop|Qt::AlignLeft,
					  index.data(Qt::DisplayRole).toString(),
					  nullptr);

	// Draw info
	font.setPixelSize(12);
	painter->setFont(font);
	painter->drawText(option.rect.x()+48,
					  option.rect.y()+36,
					  option.rect.width()-48,
					  option.rect.height()-32,
					  Qt::AlignTop|Qt::AlignLeft,
					  info,
					  nullptr);
}


