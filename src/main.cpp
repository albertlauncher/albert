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

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QFileSystemWatcher>

#include <functional>

#include "mainwidget.h"
#include "settingswidget.h"
#include "settings.h"
#include "globalhotkey.h"
#include "extensionhandler.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString suffix;
    if (context.function)
        suffix = QString("  --  [%1]").arg(context.function);
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", message.toLocal8Bit().constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "\x1b[33;1mWarning:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "\x1b[31;1mCritical:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "\x1b[41;30;4mFATAL:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        abort();
    }
}

int main(int argc, char *argv[])
{

	/*
	 *  INITIALIZE APPLICATION
	 */

    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    a.setApplicationName("albert");
    a.setApplicationDisplayName("Albert");
    a.setApplicationVersion("0.6");
	a.setWindowIcon(QIcon(":app_icon"));
	a.setQuitOnLastWindowClosed(false); // Dont quit after settings close


    /*
     *  MAKE SURE THE NEEDED DIRECTORIES EXIST
     */

    {
        QDir data(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        if (!data.exists())
            data.mkpath(".");
        QDir conf(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                  +"/"+ qApp->applicationName());
        if (!conf.exists())
            conf.mkpath(".");
    }


    /*
     *  THEME
     */

    MainWidget *w = new MainWidget;

    {
        QString theme = gSettings->value(CFG_THEME, CFG_THEME_DEF).toString();
        QFileInfoList themes;
        QStringList themeDirs = QStandardPaths::locateAll(
            QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory
        );
        for (QDir d : themeDirs)
            themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
        // Find and apply the theme
        bool success = false;
        for (QFileInfo fi : themes){
            if (fi.baseName() == theme) {
                QFile f(fi.canonicalFilePath());
                if (f.open(QFile::ReadOnly)) {
                    qApp->setStyleSheet(f.readAll());
                    f.close();
                    success = true;
                    break;
                }
            }
        }
        if (!success) {
            qFatal("FATAL: Stylefile not found: %s", theme.toStdString().c_str());
            exit(EXIT_FAILURE);
        }
    }


    /*
     *  HOTKEY
     */

    // Albert without hotkey is useless. Force it!
    gHotkeyManager->registerHotkey(gSettings->value(CFG_HOTKEY, CFG_HOTKEY_DEF).toString());
    if (gHotkeyManager->hotkeys().empty()) {
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is invalid, please set it. Press ok to "
                           "open the settings or press close to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ){
            gHotkeyManager->disable();
            SettingsWidget *sw = new SettingsWidget(w);
            QObject::connect(sw, &QWidget::destroyed,
                             gHotkeyManager, &GlobalHotkey::enable);
            sw->ui.tabs->setCurrentIndex(0);
            sw->show();
        }
        else
            exit(0);
    }


    /*
     *  SETUP SIGNAL FLOW
     */

    ExtensionHandler      extensionHandler;
    extensionHandler.initialize();

    // Show mainwidget if hotkey is pressed
    QObject::connect(gHotkeyManager, &GlobalHotkey::hotKeyPressed,
                     w, &MainWidget::toggleVisibility);

    // Setup and teardown query sessions with the state of the widget
    QObject::connect(w, &MainWidget::widgetShown,
                     &extensionHandler, &ExtensionHandler::setupSession);
    QObject::connect(w, &MainWidget::widgetHidden,
                     &extensionHandler, &ExtensionHandler::teardownSession);

    // Click on _settingsButton (or shortcut) closes albert + opens settings dialog
    QObject::connect(w->ui.inputLine->_settingsButton, &QPushButton::clicked,
                     w, &MainWidget::hide);
    QObject::connect(w->ui.inputLine->_settingsButton, &QPushButton::clicked,
                     gHotkeyManager, &GlobalHotkey::disable);
    QObject::connect(w->ui.inputLine->_settingsButton, &QPushButton::clicked,
                     [&](){
                            SettingsWidget *sw = new SettingsWidget(w);
                            QObject::connect(sw, &QWidget::destroyed,
                                             gHotkeyManager, &GlobalHotkey::enable);
                            sw->show();
                          });

    // A change in text triggers requests
    QObject::connect(w->ui.inputLine, &QLineEdit::textChanged,
                     &extensionHandler, &ExtensionHandler::startQuery);

    // Make the list show the results of the current query
    QObject::connect(&extensionHandler, &ExtensionHandler::currentQueryChanged,
                     w->ui.proposalList, &ProposalListView::setModel);


    /*
     *  DESERIALIZATION
     */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/history.dat");
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        QDataStream in(&f);
        QStringList SL;
        in >> SL;
        w->ui.inputLine->setHistory(SL.toStdList());
        f.close();
    } else qWarning() << "Could not open file" << f.fileName();


    /*
     *  E N T E R   T H E   L O O P
     */

    int ret = a.exec();


    /*
     *  SERIALIZATION
     */
    f.setFileName(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/history.dat");
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)){
        QDataStream out( &f );
        qDebug() << QStringList::fromStdList(w->ui.inputLine->getHistory());
        out << QStringList::fromStdList(w->ui.inputLine->getHistory());
        f.close();
    } else qCritical() << "Could not write to " << f.fileName();


    /*
     *  CLEANUP
     */
    extensionHandler.finalize();
    gSettings->sync();
    return ret;
}
