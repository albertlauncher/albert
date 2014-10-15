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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "singleton.h"
#include "ui_settingsdialog.h"
#include <QWidget>

class SettingsDialog : public QDialog, public Singleton<SettingsDialog>
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = 0);

protected:
	friend class Singleton<SettingsDialog>;
	Ui::SettingsDialog ui;

protected slots:
	void onSkinClicked(QListWidgetItem*);


};

#endif // SETTINGSDIALOG_H
