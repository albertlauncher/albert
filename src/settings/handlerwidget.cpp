// Copyright (c) 2022-2024 Manuel Schneider

#include "handlermodel.h"
#include "handlerwidget.h"
#include <QHeaderView>
using namespace albert;

HandlerWidget::HandlerWidget(QueryEngine &qe, ExtensionRegistry &er, QWidget *parent)
    : QTableView(parent)
{
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->hide();

    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsClickable(false);

    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    setAlternatingRowColors(true);
    QAbstractTableModel *model;
    setModel(model = new HandlerModel(qe, er, this)); // Takes ownership
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QTableView::DoubleClicked|QTableView::SelectedClicked|QTableView::EditKeyPressed);


    // Select first selectable item
    // CAUTION: returns!
    for (int row = 0; row < model->rowCount(); ++row)
        for (int col = 0; col < model->columnCount(); ++col)
            if (auto index = model->index(row, col); index.flags() & Qt::ItemIsEnabled) {
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
                return;
            }
}

HandlerWidget::~HandlerWidget() = default;
