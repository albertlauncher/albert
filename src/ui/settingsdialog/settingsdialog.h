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

#include "ui_settingsdialog.h"
#include "singleton.h"
#include "hotkeywidget.h"

class MainWidget;

class SettingsWidget : public QWidget
{
	Q_OBJECT
	Ui::SettingsDialog ui;
	HotkeyWidget* _hkWidget;
	MainWidget* _mainWidget;

public:
	enum class Tab{
		General,
		Appearance,
		Modules,
		About
	};

	explicit SettingsWidget(MainWidget *ref);

protected:
	void closeEvent(QCloseEvent * event) override;

protected slots:
	void onThemeChanged(int);
	void onNItemsChanged(int i);
	void onSubModeSelChanged(int);
	void onSubModeDefChanged(int);
	void modActionCtrlChanged(int);
	void modActionMetaChanged(int);
	void modActionAltChanged(int);

public slots:
	void show();
	void show(Tab);
};

#endif // SETTINGSDIALOG_H
