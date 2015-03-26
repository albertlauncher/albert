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
#include <QSortFilterProxyModel>
#include <QDebug>

#include "mainwidget.h"
#include "extensionhandler.h"
#include "settingsdialog.h"
#include "settings.h"
#include "globalhotkey.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		abort();
	}
}

int main(int argc, char *argv[])
{
//	qInstallMessageHandler(myMessageOutput);

	/*
	 *  INITIALIZE APPLICATION
	 */

    QApplication          a(argc, argv);
	QCoreApplication::setApplicationName(QString::fromLocal8Bit("albert"));
	a.setWindowIcon(QIcon(":app_icon"));
	a.setQuitOnLastWindowClosed(false); // Dont quit after settings close

    MainWidget            mw;
    QSortFilterProxyModel sortProxyModel;
	ExtensionHandler      extensionHandler;

	extensionHandler.initialize();
    mw._proposalListView->setModel(&sortProxyModel);

	/*
	 *  THEME
	 */

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
					 &sortProxyModel, &QSortFilterProxyModel::setSourceModel);


	/*
	 *  E N T E R   T H E   L O O P
	 */

	return a.exec();

	/*
	 *  CLEANUP
	 */
	extensionHandler.finalize();
    gSettings->sync();
}
