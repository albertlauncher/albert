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

class MainWidget;

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:
	enum class Tab{
		General,
		Appearance,
		Modules,
		About
	};

	explicit SettingsWidget(MainWidget *ref);
    void show();
    void show(Tab);

private:
	void grabAll();
	void releaseAll();
    void updatePluginList();
    void updatePluginInformations();
    void onThemeChanged(int);
    void onPbHotkeyPressed();
    void onHotkeyChanged(int);
    void onShowCenteredChanged(bool);
    void onMaxHistoryChanged(int);
    //	void onNItemsChanged(int i);
    //	void onSubModeSelChanged(int);
    //	void onSubModeDefChanged(int);
    //	void modActionCtrlChanged(int);
    //	void modActionMetaChanged(int);
    //	void modActionAltChanged(int);
    void closeEvent(QCloseEvent * event) override;
    void keyPressEvent (QKeyEvent *) override;
    void keyReleaseEvent ( QKeyEvent* ) override;

    Ui::SettingsDialog ui;
    MainWidget* _mainWidget;
    bool _waitingForHotkey;
};

#endif // SETTINGSDIALOG_H
