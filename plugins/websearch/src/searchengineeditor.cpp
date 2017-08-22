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
#include <QStandardPaths>
#include "searchengineeditor.h"



/** ***************************************************************************/
Websearch::SearchEngineEditor::SearchEngineEditor(const SearchEngine &searchEngine, QWidget *parent)
    : QDialog(parent), searchEngine_(searchEngine) {

    ui.setupUi(this);
    setWindowModality(Qt::WindowModal);

    ui.lineEdit_name->setText(searchEngine.name);
    ui.lineEdit_trigger->setText(searchEngine.trigger);
    ui.lineEdit_url->setText(searchEngine.url);
    ui.toolButton_icon->setIcon(QIcon(searchEngine.iconPath));

    connect(ui.lineEdit_name, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.name = text; });

    connect(ui.lineEdit_trigger, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.trigger = text; });

    connect(ui.lineEdit_url, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.url = text; });

    connect(ui.toolButton_icon, &QToolButton::clicked,
            [this](){

        QString fileName =
                QFileDialog::getOpenFileName(
                    this,
                    tr("Choose icon"),
                    QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                    tr("Images (*.png *.svg)"));

        if(fileName.isEmpty())
            return;

        searchEngine_.iconPath = fileName;
        ui.toolButton_icon->setIcon(QIcon(fileName));
    });

}
