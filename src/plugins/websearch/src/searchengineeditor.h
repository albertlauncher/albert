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

#pragma once
#include <QDialog>
#include "ui_searchengineeditor.h"
#include "searchengine.h"

namespace Websearch {

class SearchEngineEditor : public QDialog
{
    Q_OBJECT

public:

    explicit SearchEngineEditor(const SearchEngine &searchEngine, QWidget *parent = 0);
    const SearchEngine &searchEngine() { return searchEngine_; }

private:

    SearchEngine searchEngine_;
    Ui::SearchEngineEditor ui;

};

}
