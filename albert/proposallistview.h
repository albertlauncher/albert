#ifndef PROPOSALLISTVIEW_H
#define PROPOSALLISTVIEW_H

#include <QListWidget>
#include <QEvent>
#include <QKeyEvent>

class ProposalListView: public QListView
{
	Q_OBJECT

	int _nItemsToShow;

public:
	explicit ProposalListView(QWidget *parent = 0);

protected:
	bool eventFilter(QObject*, QEvent *event);
};

#endif // PROPOSALLISTVIEW_H
