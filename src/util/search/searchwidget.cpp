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

#include "searchwidget.h"
#include "ui_searchwidget.h"
#include "wordmatchsearch.h"
#include "fuzzysearch.h"

SearchWidget::SearchWidget(AbstractIndex *ref) : _ref(ref)
{
	/* SETUP UI*/

	ui.setupUi(this);
	QString regexp = QString("^\\%1?\\d$").arg(QLocale().decimalPoint());
	val.setRegExp(QRegExp(regexp));
	ui.errorToleranceLineEdit->setValidator(&val);


	/* INIT UI*/

	// Initialize
	const FuzzySearch* fs = dynamic_cast<const FuzzySearch*>(_ref->search());
	bool isFuzzy = (fs != nullptr);
	ui.fuzzySearchCheckBox->setChecked(isFuzzy);
	ui.sizeOfQGramsSpinBox->setEnabled(isFuzzy);
	ui.sizeOfQGramsLabel->setEnabled(isFuzzy);
	ui.errorToleranceLineEdit->setEnabled(isFuzzy);
	ui.errorToleranceLabel->setEnabled(isFuzzy);
	if (isFuzzy) {
		ui.sizeOfQGramsSpinBox->setValue( fs->q() );
		ui.errorToleranceLineEdit->setText( QLocale().toString( fs->delta() ) );
	}


	/* SETUP SIGNALS */

	// Inline oneliners
	// Change the searchtype on selection TODO
	connect(ui.fuzzySearchCheckBox, &QCheckBox::clicked, [&](bool b){
		if (b){
			_ref->setSearch(new FuzzySearch(_ref, 3, 2));
			ui.sizeOfQGramsSpinBox->setValue(3);
			ui.errorToleranceLineEdit->setText("2");
		}
		else
			_ref->setSearch(new WordMatchSearch());
	});

	// Change the searchindex on qgram change
	connect(ui.sizeOfQGramsSpinBox, &QSpinBox::editingFinished, [&](){
		// ASSUMED THAT THIS IS ONLY ENABLED IF SEARCH IS FUZZY
		dynamic_cast<FuzzySearch*>(_ref->_search)->setQ(ui.sizeOfQGramsSpinBox->value());
	});

	// Change the searchindex on delta change
	connect(ui.errorToleranceLineEdit, &QLineEdit::editingFinished, [&](){
		// ASSUMED THAT THIS IS ONLY ENABLED IF SEARCH IS FUZZY
		dynamic_cast<FuzzySearch*>(_ref->_search)->setDelta(QLocale().toDouble(ui.errorToleranceLineEdit->text()));
	});
}
