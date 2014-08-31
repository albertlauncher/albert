#include "proposallistwidget.h"
#include "proposallistdelegate.h"

ProposalListWidget::ProposalListWidget(QWidget *parent) :
	QListWidget(parent)
{
	_nItemsToShow = 5;
	setItemDelegate(new ProposalListDelegate);
	setObjectName(QString::fromLocal8Bit("ProposalListWidget"));
}
