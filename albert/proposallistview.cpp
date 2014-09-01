#include "proposallistview.h"
#include "proposallistdelegate.h"

ProposalListView::ProposalListView(QWidget *parent) :
	QListView(parent)
{
	_nItemsToShow = 8;
	setItemDelegate(new ProposalListDelegate);
	setObjectName(QString::fromLocal8Bit("ProposalListWidget"));
}

bool ProposalListView::eventFilter(QObject*, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down
				|| keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_PageUp){
			this->keyPressEvent(keyEvent);
			return true;
		}
	}
	return false;
}
