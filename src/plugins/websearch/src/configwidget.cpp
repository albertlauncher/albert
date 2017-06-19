// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "configwidget.h"
#include "enginesmodel.h"
#include "extension.h"
#include "searchengineeditor.h"

/** ***************************************************************************/
Websearch::ConfigWidget::ConfigWidget(Extension *extension, QWidget *parent)
    : QWidget(parent), extension_(extension) {

    ui.setupUi(this);

    enginesModel_ = new EnginesModel(extension, ui.tableView_searches);
    ui.tableView_searches->setModel(enginesModel_);

    ui.tableView_searches->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView_searches->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Initialize connections
    connect(ui.pushButton_new, &QPushButton::clicked,
            this, &ConfigWidget::onButton_new);

    connect(ui.pushButton_remove, &QPushButton::clicked,
            this, &ConfigWidget::onButton_remove);

    connect(ui.pushButton_restoreDefaults, &QPushButton::clicked,
            this, &ConfigWidget::onButton_restoreDefaults);

    connect(ui.tableView_searches, &QTableView::activated,
            this, &ConfigWidget::onActivated);
}



/** ***************************************************************************/
Websearch::ConfigWidget::~ConfigWidget() {
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onActivated(QModelIndex index) {
    int row = index.row();
    SearchEngineEditor searchEngineEditor(extension_->engines()[static_cast<ulong>(row)], this);

    if (searchEngineEditor.exec()){
        // Set the new engine
        const SearchEngine & searchEngine = searchEngineEditor.searchEngine();
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.name, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.iconPath, Qt::DecorationRole);
        enginesModel_->setData(enginesModel_->index(row, 1), searchEngine.trigger, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 2), searchEngine.url, Qt::DisplayRole);
    }
    ui.tableView_searches->reset();
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_new() {

    // Open search engine editor
    SearchEngine searchEngine;
    searchEngine.iconPath = ":default";
    SearchEngineEditor searchEngineEditor(searchEngine, this);

    if (searchEngineEditor.exec()){

        // Insert new row in model
        int row = (ui.tableView_searches->currentIndex().isValid())
                ? ui.tableView_searches->currentIndex().row()
                : ui.tableView_searches->model()->rowCount();
        enginesModel_->insertRow(row);

        // Set the new engine
        searchEngine = searchEngineEditor.searchEngine();
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.name, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.iconPath, Qt::DecorationRole);
        enginesModel_->setData(enginesModel_->index(row, 1), searchEngine.trigger, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 2), searchEngine.url, Qt::DisplayRole);

        // Set current
        QModelIndex index = ui.tableView_searches->model()->index(row, 0, QModelIndex());
        ui.tableView_searches->setCurrentIndex(index);
    }
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_remove() {
    // Ask if sure
    int row = ui.tableView_searches->currentIndex().row();
    QString engineName = ui.tableView_searches->model()
            ->data(ui.tableView_searches->model()->index(row, 1)).toString();
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Sure?",
                                  QString("Do you really want to remove '%1' from the search engines?")
                                  .arg(engineName),
                                  QMessageBox::Yes|QMessageBox::No);
    // Remove if sure
    if (reply == QMessageBox::Yes)
        ui.tableView_searches->model()->removeRow(ui.tableView_searches->currentIndex().row());
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_restoreDefaults() {
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Sure?",
                                  QString("Do you really want to restore the default search engines?"),
                                  QMessageBox::Yes|QMessageBox::No);
    // Remove if sure
    if (reply == QMessageBox::Yes)
        enginesModel_->restoreDefaults();
}
