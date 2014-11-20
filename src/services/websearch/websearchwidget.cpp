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

#include "websearchwidget.h"
#include "websearchitem.h"

#include <QFileDialog>
#include <QStandardPaths>

/**************************************************************************/
WebSearchWidget::WebSearchWidget(WebSearch *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	ui.setupUi(this);
	updateUI();

	// Initialize connections
	connect(ui.pb_restoreDefaults, &QPushButton::clicked, this, &WebSearchWidget::resetDefaults);
	connect(ui.pb_new, &QPushButton::clicked, this, &WebSearchWidget::onButton_new);
	connect(ui.pb_setIcon, &QPushButton::clicked, this, &WebSearchWidget::onButton_setIcon);
	connect(ui.pb_remove, &QPushButton::clicked, this, &WebSearchWidget::onButton_remove);
	connect(ui.tableWidget_searches, &QTableWidget::cellChanged, this, &WebSearchWidget::onChange);
}

/**************************************************************************/
void WebSearchWidget::onButton_new()
{
	_ref->_searchEngines.push_back(new WebSearch::Item);
	int r = ui.tableWidget_searches->rowCount();
	ui.tableWidget_searches->insertRow(r);
	ui.tableWidget_searches->setItem(r,0,new QTableWidgetItem("<Name>"));
	ui.tableWidget_searches->setItem(r,1,new QTableWidgetItem("<Shortcut>"));
	ui.tableWidget_searches->setItem(r,2,new QTableWidgetItem("<Url containing %s>"));
	ui.tableWidget_searches->resizeColumnsToContents();
	ui.tableWidget_searches->resizeRowsToContents();
}

/**************************************************************************/
void WebSearchWidget::onButton_remove()
{
	if (ui.tableWidget_searches->currentRow() < 0)
		return;

	delete _ref->_searchEngines[ui.tableWidget_searches->currentRow()];
	_ref->_searchEngines.remove(ui.tableWidget_searches->currentRow());
	ui.tableWidget_searches->removeRow(ui.tableWidget_searches->currentRow());
	ui.tableWidget_searches->resizeColumnsToContents();
	ui.tableWidget_searches->resizeRowsToContents();
}

/**************************************************************************/
void WebSearchWidget::onButton_setIcon()
{
	int row = ui.tableWidget_searches->currentRow();
	if (row < 0)
		return;

	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Choose icon"),
													QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
													tr("Images (*.png *.svg *.jpg)"));
	if(fileName.isEmpty())
		return;

	_ref->_searchEngines[row]->_iconPath = fileName;
	ui.tableWidget_searches->item(row,0)->setIcon(QIcon(fileName));
	ui.tableWidget_searches->resizeColumnsToContents();
	ui.tableWidget_searches->resizeRowsToContents();
}

/**************************************************************************/
void WebSearchWidget::onChange(int row , int col)
{
	switch (col) {
	case 0:
		_ref->_searchEngines[row]->_name = ui.tableWidget_searches->item(row,col)->text();
		break;
	case 1:
		_ref->_searchEngines[row]->_shortcut = ui.tableWidget_searches->item(row,col)->text();
		break;
	case 2:
		_ref->_searchEngines[row]->_url = ui.tableWidget_searches->item(row,col)->text();
		break;
	default:// Does never happen
		break;
	}
	ui.tableWidget_searches->resizeColumnsToContents();
	ui.tableWidget_searches->resizeRowsToContents();
}

/**************************************************************************/
void WebSearchWidget::resetDefaults()
{
	_ref->restoreDefaults();
	updateUI();
}

/**************************************************************************/
void WebSearchWidget::updateUI()
{
	ui.tableWidget_searches->clearContents();

	ui.tableWidget_searches->setRowCount(_ref->_searchEngines.size());
	for (int i = 0; i < _ref->_searchEngines.size(); ++i)
	{
		ui.tableWidget_searches->setItem(i,0,new QTableWidgetItem(
											 QIcon(_ref->_searchEngines[i]->_iconPath),
											 _ref->_searchEngines[i]->_name
											 ));
		ui.tableWidget_searches->setItem(i,1,new QTableWidgetItem(
											 _ref->_searchEngines[i]->_shortcut
											 ));
		ui.tableWidget_searches->setItem(i,2,new QTableWidgetItem(
											 _ref->_searchEngines[i]->_url
											 ));
	}
	ui.tableWidget_searches->resizeColumnsToContents();
	ui.tableWidget_searches->resizeRowsToContents();
}
