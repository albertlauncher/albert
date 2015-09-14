// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QListView>
#include <QEvent>
#include <QKeyEvent>
#include <QArrayData>
#include <QSettings>
#include "itemdelegate.h"

class ProposalList final: public QListView
{
    Q_OBJECT

public:
    explicit ProposalList(QWidget *parent = 0);
    ~ProposalList();
    QSize sizeHint() const override;
    void reset() override;

    void setShowInfo(bool);
    void setShowAction(bool);
    void setMaxItems(uint);

    bool showInfo() const;
    bool showAction() const;
    bool maxItems() const;



private:
    bool eventFilter(QObject*, QEvent *event) override;

    ItemDelegate *_itemDelegate;
    uint _maxItems;

    static const constexpr char* CFG_SHOW_INFO         = "showInfo";
    static const constexpr bool  CFG_SHOW_INFO_DEF     = true;
    static const constexpr char* CFG_SHOW_ACTION       = "showInfo";
    static const constexpr bool  CFG_SHOW_ACTION_DEF   = true;
    static const constexpr char* CFG_MAX_PROPOSALS     = "itemCount";
    static const constexpr uint  CFG_MAX_PROPOSALS_DEF = 5;
};
