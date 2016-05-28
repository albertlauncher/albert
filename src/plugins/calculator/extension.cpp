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

#include <QDebug>
#include <QClipboard>
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "xdgiconlookup.h"
#include "muParser.h"
#include "albertapp.h"

const QString Calculator::Extension::CFG_SEPS      = "group_separators";
const bool    Calculator::Extension::CFG_SEPS_DEF  = false;

/** ***************************************************************************/
Calculator::Extension::Extension() : IExtension("Calculator") {
    qDebug("[%s] Initialize extension", name_);

    // Load settings
    qApp->settings()->beginGroup(name_);
    loc_.setNumberOptions(
                (qApp->settings()->value(CFG_SEPS, CFG_SEPS_DEF).toBool())
                ? loc_.numberOptions() & ~QLocale::OmitGroupSeparator
                : loc_.numberOptions() | QLocale::OmitGroupSeparator );
    qApp->settings()->endGroup();

    QString iconPath = XdgIconLookup::instance()->themeIconPath("calc", QIcon::themeName());
    iconPath_ = iconPath.isNull() ? ":calc" : iconPath;

    parser_ = new mu::Parser;

    parser_->SetDecSep(loc_.decimalPoint().toLatin1());
    parser_->SetThousandsSep(loc_.groupSeparator().toLatin1());

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
Calculator::Extension::~Extension() {
    qDebug("[%s] Finalize extension", name_);

    // Save settings
    qApp->settings()->beginGroup(name_);
    qApp->settings()->setValue(CFG_SEPS, !loc_.numberOptions().testFlag(QLocale::OmitGroupSeparator));
    qApp->settings()->endGroup();

    delete parser_;

    qDebug("[%s] Extension finalized", name_);
}



/** ***************************************************************************/
QWidget *Calculator::Extension::widget(QWidget *parent) {
    if (widget_.isNull()){
        widget_ = new ConfigWidget(parent);

        widget_->ui.checkBox_groupsep->setChecked(!(loc_.numberOptions() & QLocale::OmitGroupSeparator));
        connect(widget_->ui.checkBox_groupsep, &QCheckBox::toggled, [this](bool checked){
            this->loc_.setNumberOptions( (checked) ? this->loc_.numberOptions() & ~QLocale::OmitGroupSeparator
                                                  : this->loc_.numberOptions() | QLocale::OmitGroupSeparator );
        });
    }
    return widget_;
}



/** ***************************************************************************/
void Calculator::Extension::handleQuery(shared_ptr<Query> query) {
    parser_->SetExpr(query->searchTerm().toLower().toStdString());
    QString result;
    try {
        result = loc_.toString(parser_->Eval(), 'G', 16);
    }
    catch (mu::Parser::exception_type &e) {
      return;
    }

    std::shared_ptr<StandardItem> calcItem = std::make_shared<StandardItem>();
    calcItem->setText(result);
    calcItem->setSubtext(QString("Result of '%1'").arg(query->searchTerm()));
    calcItem->setIcon(iconPath_);
    calcItem->setAction([result](){
        QApplication::clipboard()->setText(result);
    });
    query->addMatch(calcItem, SHRT_MAX);
}
