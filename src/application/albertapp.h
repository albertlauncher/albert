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

#pragma once
#include <QApplication>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QSettings>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <vector>
using std::vector;
class MainWindow;
class HotkeyManager;
class ExtensionManager;
class QueryPrivate;
class SettingsWidget;

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<AlbertApp*>(QCoreApplication::instance()))

class AlbertApp final : public QApplication
{

    Q_OBJECT

public:

    AlbertApp(int &argc, char *argv[]);
    ~AlbertApp();

    int exec();

    // Global facade. Acessible to all subsystems
    void openSettings();
    void showWidget();
    void hideWidget();
    void clearInput();
    QSettings *settings();

    QString term();
    void setTerm(const QString&);

    void enableTrayIcon(bool enable = true);
    bool trayIconEnabled();

private:

    void onWidgetShown();
    void onWidgetHidden();
    void onInputChanged(const QString &searchTerm);

    MainWindow               *mainWindow_;
    HotkeyManager            *hotkeyManager_;
    ExtensionManager         *extensionManager_;
    QueryPrivate             *currentQuery_;
    vector<QueryPrivate*>    oldQueries_;

    QPointer<SettingsWidget> settingsWidget_;
    QLocalServer             *localServer_;
    QSettings                *settings_;
    QString                  terminal_;
    QSystemTrayIcon          *trayIcon_;
    QMenu                    *trayIconMenu_;

    static const char* CFG_TERM;
    static const char* DEF_TERM;
    static const char* CFG_SHOWTRAY;
    static const bool  DEF_SHOWTRAY;
};



