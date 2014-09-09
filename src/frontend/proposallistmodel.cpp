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

#include "proposallistmodel.h"

#include <QDebug>
#include <QIcon>



/**************************************************************************//**
 * @brief ProposalListModel::ProposalListModel
 * @param parent
 */
ProposalListModel::ProposalListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}

/**************************************************************************//**
 * @brief ProposalListModel::set
 * @param d
 */
void ProposalListModel::set(const std::vector<AbstractServiceProvider::AbstractItem *> &d){
	beginResetModel();
	_data = d;
	endResetModel();
}

/**************************************************************************//**
 * @brief ProposalListModel::clear
 */
void ProposalListModel::clear()
{
	beginResetModel();
	_data.clear();
	endResetModel();
}

/**************************************************************************//**
 * @brief ProposalListModel::action
 * @param index
 * @return
 */
void ProposalListModel::action(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(AbstractServiceProvider::AbstractItem::Action::Enter);
}

/**************************************************************************//**
 * @brief ProposalListModel::altAction
 * @param index
 */
void ProposalListModel::altAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(AbstractServiceProvider::AbstractItem::Action::Alt);
}

/**************************************************************************//**
 * @brief ProposalListModel::ctrlAction
 * @param index
 */
void ProposalListModel::ctrlAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(AbstractServiceProvider::AbstractItem::Action::Ctrl);
}

/**************************************************************************//**
 * @brief ProposalListModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ProposalListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	//	EnterText = Qt::UserRole;
	//	AltText  = Qt::UserRole+1;
	//	CtrlText  = Qt::UserRole+2;
	//	InfoText  = Qt::UserRole+3;
	//	Completion = Qt::UserRole+4;

	if (role == Qt::DisplayRole)
		return QString::fromStdString(_data[index.row()]->title());

	if (role == Qt::DecorationRole)  {
		QIcon i;
		i = QIcon::fromTheme(QString::fromStdString(_data[index.row()]->iconName()));
		if (i.isNull())
			i = QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
		return i;
}

	if (role == Qt::UserRole)
		return QString::fromStdString(_data[index.row()]->actionText(AbstractServiceProvider::AbstractItem::Action::Enter));

	if (role == Qt::UserRole+1)
		return QString::fromStdString(_data[index.row()]->actionText(AbstractServiceProvider::AbstractItem::Action::Alt));

	if (role == Qt::UserRole+2 )
		return QString::fromStdString(_data[index.row()]->actionText(AbstractServiceProvider::AbstractItem::Action::Ctrl));

	if (role == Qt::UserRole+3)
		return QString::fromStdString(_data[index.row()]->infoText());

	if (role == Qt::UserRole+4)
		return QString::fromStdString(_data[index.row()]->complete());


	return QVariant();

}

/**************************************************************************//**
 * @brief ProposalListModel::rowCount
 * @return
 */
int ProposalListModel::rowCount(const QModelIndex&) const
{
	return _data.size();
}

