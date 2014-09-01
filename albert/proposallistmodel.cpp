#include "proposallistmodel.h"

ProposalListModel::ProposalListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}

void ProposalListModel::set(std::vector<const Items::AbstractItem *> d){
	beginResetModel();
	_data = d;
	endResetModel();
}

void ProposalListModel::clear()
{
	beginResetModel();
	_data.clear();
	endResetModel();
}

QVariant ProposalListModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
		return _data[index.row()]->name();
	if (role == Qt::UserRole)
		return _data[index.row()]->info();
	return QVariant();
}

int ProposalListModel::rowCount(const QModelIndex&) const
{
	return _data.size();
}
