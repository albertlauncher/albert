// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include "propertyeditor.h"
#include "mainwindow.h"
#include "colordialog.hpp"
#include <QStyledItemDelegate>
#include <QItemEditorFactory>
#include <QTableView>
#include <QAbstractTableModel>
#include <QVBoxLayout>
#include <QHeaderView>

namespace QmlBoxModel {

class PropertyModel final : public QAbstractTableModel
{
public:
    PropertyModel(MainWindow *mainWindow, QObject * parent = 0)
        : QAbstractTableModel(parent), mainWindow_(mainWindow){
        properties_ = mainWindow_->settableProperties();
    }

    ~PropertyModel() {

    }

    int rowCount(const QModelIndex & parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return properties_.count();
    }

    int columnCount(const QModelIndex & parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 2;
    }

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (index.column()==0)
                return properties_[index.row()];
            else if (index.column()==1)
                return mainWindow_->property(properties_.at(index.row()).toLatin1().data());
        default:
            return QVariant();
        }
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
            if (section==0)
                return "Property";
            if (section==1)
                return "Value";
        }
        return QVariant();
    }

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override {
        if (role ==  Qt::EditRole){
            mainWindow_->setProperty(properties_[index.row()].toLatin1().data(), value);
            return true;
        }
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex & index) const override {
        if (index.column()==0)
            return Qt::NoItemFlags|Qt::ItemIsEnabled;
        if (index.column()==1)
            return Qt::ItemIsEditable|Qt::ItemIsEnabled;
        return Qt::NoItemFlags;
    }

private:
    MainWindow *mainWindow_;
    QStringList properties_;
};

}



///****************************************************************************/
QmlBoxModel::PropertyEditor::PropertyEditor(MainWindow *mainWindow, QWidget *parent) :
    QDialog (parent){

    resize(300, 300);
    setWindowTitle("PropertyEditor");
//    setWindowFlags(windowFlags());

    // Set ownership
    QTableView *tableView = new QTableView(this);
    tableView->setObjectName(QStringLiteral("tableView"));
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setAlternatingRowColors(true);
    tableView->setShowGrid(false);
    tableView->horizontalHeader()->setMinimumSectionSize(16);
    // Set model and ownership
    tableView->setModel(new PropertyModel(mainWindow, tableView));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    QItemEditorFactory *editorFactory = new QItemEditorFactory;
    editorFactory->registerEditor(QVariant::Color, new QStandardItemEditorCreator<ColorDialog>());

    // Create a delgate using the factory
    QStyledItemDelegate *delegate = new QStyledItemDelegate(this);
    delegate->setItemEditorFactory(editorFactory);
    tableView->setItemDelegate(delegate);
}
