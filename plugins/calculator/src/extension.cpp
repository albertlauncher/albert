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

#include <QClipboard>
#include <QDebug>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "muParser.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
#include "xdg/iconlookup.h"
using std::vector;
using std::shared_ptr;
using namespace Core;



namespace {
const QString CFG_SEPS      = "group_separators";
const bool    CFG_SEPS_DEF  = false;
}



class Calculator::Private
{
public:
    QPointer<ConfigWidget> widget;
    std::unique_ptr<mu::Parser> parser;
    QLocale locale;
    QString iconPath;
};



/** ***************************************************************************/
Calculator::Extension::Extension()
    : Core::Extension("org.albert.extension.calculator"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private){

    // FIXME Qt6 Workaround for https://bugreports.qt.io/browse/QTBUG-58504
    d->locale = QLocale(QLocale::system().name());

    // Load settings
    d->locale.setNumberOptions(
                (settings().value(CFG_SEPS, CFG_SEPS_DEF).toBool())
                ? d->locale.numberOptions() & ~QLocale::OmitGroupSeparator
                : d->locale.numberOptions() | QLocale::OmitGroupSeparator );

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->parser.reset(new mu::Parser);
    d->parser->SetDecSep(d->locale.decimalPoint().toLatin1());
    d->parser->SetThousandsSep(d->locale.groupSeparator().toLatin1());
}



/** ***************************************************************************/
Calculator::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Calculator::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        d->widget->ui.checkBox_groupsep->setChecked(!(d->locale.numberOptions() & QLocale::OmitGroupSeparator));
        connect(d->widget->ui.checkBox_groupsep, &QCheckBox::toggled, [this](bool checked){
            settings().setValue(CFG_SEPS, checked);
            d->locale.setNumberOptions( (checked) ? d->locale.numberOptions() & ~QLocale::OmitGroupSeparator
                                                  : d->locale.numberOptions() | QLocale::OmitGroupSeparator );
        });
    }
    return d->widget;
}



/** ***************************************************************************/
void Calculator::Extension::handleQuery(Core::Query * query) const {

    if ( query->searchTerm().isEmpty() )
        return;

    d->parser->SetExpr(query->searchTerm().toLower().toStdString());
    QString result;
    try {
        result = d->locale.toString(d->parser->Eval(), 'G', 16);
    }
    catch (mu::Parser::exception_type &e) {
      return;
//     http://beltoforion.de/article.php?a=muparser&p=errorhandling
//      ecUNEXPECTED_OPERATOR	0	Unexpected binary operator found
//      ecUNASSIGNABLE_TOKEN	1	Token cant be identified
//      ecUNEXPECTED_EOF	2	Unexpected end of formula. (Example: "2+sin(")
//      ecUNEXPECTED_ARG_SEP	3	An unexpected argument separator has been found. (Example: "1,23")
//      ecUNEXPECTED_ARG	4	An unexpected argument has been found
//      ecUNEXPECTED_VAL	5	An unexpected value token has been found
//      ecUNEXPECTED_VAR	6	An unexpected variable token has been found
//      ecUNEXPECTED_PARENS	7	Unexpected parenthesis, opening or closing
//      ecUNEXPECTED_STR	8	A string has been found at an inapropriate position
//      ecSTRING_EXPECTED	9	A string function has been called with a different type of argument
//      ecVAL_EXPECTED	10	A numerical function has been called with a non value type of argument
//      ecMISSING_PARENS	11	Missing parens. (Example: "3*sin(3")
//      ecUNEXPECTED_FUN	12	Unexpected function found. (Example: "sin(8)cos(9)")
//      ecUNTERMINATED_STRING	13	unterminated string constant. (Example: "3*valueof("hello)")
//      ecTOO_MANY_PARAMS	14	Too many function parameters
//      ecTOO_FEW_PARAMS	15	Too few function parameters. (Example: "ite(1<2,2)")
//      ecOPRT_TYPE_CONFLICT	16	binary operators may only be applied to value items of the same type
//      ecSTR_RESULT	17	result is a string
//      ecINVALID_NAME	18	Invalid function, variable or constant name.
//      ecINVALID_BINOP_IDENT	19	Invalid binary operator identifier.
//      ecINVALID_INFIX_IDENT	20	Invalid infix operator identifier.
//      ecINVALID_POSTFIX_IDENT	21	Invalid postfix operator identifier.
//      ecBUILTIN_OVERLOAD	22	Trying to overload builtin operator
//      ecINVALID_FUN_PTR	23	Invalid callback function pointer
//      ecINVALID_VAR_PTR	24	Invalid variable pointer
//      ecEMPTY_EXPRESSION	25	The expression string is empty
//      ecNAME_CONFLICT	26	Name conflict
//      ecOPT_PRI	27	Invalid operator priority
//      ecDOMAIN_ERROR	28	catch division by zero, sqrt(-1), log(0) (currently unused)
//      ecDIV_BY_ZERO	29	Division by zero (currently unused)
//      ecGENERIC	30	Error that does not fit any other code but is not an internal error
//      ecLOCALE	31	Conflict with current locale
//      ecUNEXPECTED_CONDITIONAL	32	Unexpected if then else operator
//      ecMISSING_ELSE_CLAUSE	33	Missing else clause
//      ecMISPLACED_COLON	34	Misplaced colon
//      ecINTERNAL_ERROR	35	Internal error of any kind.
    }

    shared_ptr<StandardItem> calcItem = std::make_shared<StandardItem>("muparser-eval");
    calcItem->setText(result);
    calcItem->setSubtext(QString("Result of '%1'").arg(query->searchTerm()));
    calcItem->setIconPath(d->iconPath);

    // Build actions
    vector<shared_ptr<Action>> actions;

    shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(result));
    action->setAction([=](){
        QApplication::clipboard()->setText(result); });
    actions.push_back(action);

    // Make searchterm a lvalue that can be captured by the lambda
    QString text = query->searchTerm();
    action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(text));
    action->setAction([=](){
        QApplication::clipboard()->setText(text); });
    actions.push_back(action);

    text = QString("%1 = %2").arg(query->searchTerm(), result);
    action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(text));
    action->setAction([=](){
        QApplication::clipboard()->setText(text); });
    actions.push_back(action);

    calcItem->setActions(std::move(actions));

    query->addMatch(std::move(calcItem), UINT_MAX);
}
