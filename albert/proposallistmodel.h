#ifndef PROPOSALLISTMODEL_H
#define PROPOSALLISTMODEL_H

#include <QAbstractListModel>
#include "item.h"

class ProposalListModel : public QAbstractListModel
{
	Q_OBJECT



public:
	explicit ProposalListModel(QObject *parent = 0);
	void set(std::vector<const Items::AbstractItem *> d);
	void clear();

protected:
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex & = QModelIndex()) const;

	std::vector<const Items::AbstractItem *> _data;

};

#endif // PROPOSALLISTMODEL_H
