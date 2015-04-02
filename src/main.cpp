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
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

#include "mainwidget.h"
#include "extensionhandler.h"
#include "settingsdialog.h"
#include "settings.h"
#include "globalhotkey.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "\x1b[32m[%s]\x1b[0m %s\n", context.function, localMsg.constData());
		break;
	case QtWarningMsg:
        fprintf(stderr, "\x1b[32m[%s]\x1b[0m\x1b[33m Warning:\x1b[0m %s\n", context.function, localMsg.constData());
		break;
	case QtCriticalMsg:
        fprintf(stderr, "\x1b[32m[%s]\x1b[0m\x1b[31m Critical:\x1b[0m %s\n", context.function, localMsg.constData());
		break;
	case QtFatalMsg:
        fprintf(stderr, "\x1b[41;30;4mFATAL:\x1b[0m %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		abort();
	}
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);


	/*
	 *  INITIALIZE APPLICATION
	 */

    QApplication          a(argc, argv);
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
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

    MainWidget            mw;

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
				QFile styleFile(fi.canonicalFilePath());
				if (styleFile.open(QFile::ReadOnly)) {
					qApp->setStyleSheet(styleFile.readAll());
					styleFile.close();
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
                           "Hotkey is invalid, please set it. Press ok to "\
                           "open the settings or press close to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ){
            SettingsWidget *sw = new SettingsWidget(&mw);
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
                     &mw, &MainWidget::toggleVisibility);

	// Setup and teardown query sessions with the state of the widget
    QObject::connect(&mw, &MainWidget::widgetShown,
					 &extensionHandler, &ExtensionHandler::setupSession);
    QObject::connect(&mw, &MainWidget::widgetHidden,
					 &extensionHandler, &ExtensionHandler::teardownSession);

	// settingsDialogRequested closes albert + opens settings dialog
    QObject::connect(mw._inputLine, &InputLine::settingsDialogRequested,
                     &mw, &MainWidget::hide);
    QObject::connect(mw._inputLine, &InputLine::settingsDialogRequested,
                     [&](){
                            SettingsWidget *sw = new SettingsWidget(&mw);
                            sw->show();
                          });

	// A change in text triggers requests
    QObject::connect(mw._inputLine, &QLineEdit::textChanged,
					 &extensionHandler, &ExtensionHandler::startQuery);

	// Make the list show the results of the current query
	QObject::connect(&extensionHandler, &ExtensionHandler::currentQueryChanged,
                     mw._proposalListView, &ProposalListView::setModel);


	/*
	 *  E N T E R   T H E   L O O P
	 */

    int ret = a.exec();


	/*
	 *  CLEANUP
	 */
	extensionHandler.finalize();
    gSettings->sync();
    return ret;
}
