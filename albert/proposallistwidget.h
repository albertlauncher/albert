#ifndef PROPOSALLISTWIDGET_H
#define PROPOSALLISTWIDGET_H

#include <QListWidget>

class ProposalListWidget : public QListWidget
{
	Q_OBJECT

	int              _nItemsToShow;

public:
	explicit ProposalListWidget(QWidget *parent = 0);

signals:

public slots:

};

#endif // PROPOSALLISTWIDGET_H
