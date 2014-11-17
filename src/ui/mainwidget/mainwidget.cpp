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

#include "mainwidget.h"
#include "engine.h"
#include "settingsdialog.h"
#include <QEvent>
#include <QLabel>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>

/**************************************************************************/
MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	_settingsDialog = nullptr;

	/* INITIALIZE UI */

	setWindowTitle(QString::fromLocal8Bit("Albert"));
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool
					);

	QVBoxLayout *l2 = new QVBoxLayout;
	l2->setMargin(0);
	l2->setSizeConstraint(QLayout::SetFixedSize);
	l2->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	this->setLayout(l2);

	_frame2 = new QFrame;
	_frame2->setObjectName(QString::fromLocal8Bit("bottomframe"));
	_frame2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	l2->addWidget(_frame2);

	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	l1->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	_frame2->setLayout(l1);

	_frame1 = new QFrame;
	_frame1->setObjectName(QString::fromLocal8Bit("topframe"));
	_frame1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	l1->addWidget(_frame1);

	QVBoxLayout *contentLayout = new QVBoxLayout();
	contentLayout->setMargin(0);
	contentLayout->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	_frame1->setLayout(contentLayout);

	_inputLine = new InputLine;
	_inputLine->setObjectName(QString::fromLocal8Bit("inputline"));
	_inputLine->setContextMenuPolicy(Qt::NoContextMenu);
	_inputLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	contentLayout->addWidget(_inputLine);

	_engine = new Engine;
	_proposalListView = new ProposalListView;
	_proposalListView->setModel(_engine);
	_proposalListView->setObjectName("proposallist");
	_proposalListView->setFocusPolicy(Qt::NoFocus);
	_proposalListView->setFocusProxy(_inputLine);
	_proposalListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	_proposalListView->hide();
	// Proposallistview intercepts inputline's events (Navigation with keys, pressed modifiers, etc)
	_inputLine->installEventFilter(_proposalListView);
	contentLayout->addWidget(_proposalListView);



	/* DESERIALIZE DATA (BEFORE!! LOAD SETTINGS ) */

	QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db");
	if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
		qDebug() << "Deserializing from" << f.fileName();
		QDataStream in(&f);
		_engine->deserilizeData(in);
		_inputLine->deserilizeData(in);
		f.close();
	} else {
		qWarning() << "Could not open file" << f.fileName();
		_engine->initialize();
	}



	/* LOAD SETTINGS */

	QSettings settings(QSettings::UserScope, "albert", "albert");
	_engine->loadSettings(settings);
	_proposalListView->loadSettings(settings);
	_inputLine->loadSettings(settings);

	_showCentered = settings.value("showCentered", "true").toBool();
	_hotkeyManager.registerHotkey(settings.value("hotkey", "").toString());
	_theme = settings.value("Theme", "Standard").toString();

	// This one has to be instanciated after! all settings are loaded
	_settingsDialog = new SettingsWidget(this);

	// Albert without hotkey is useless. Force it!
	if (_hotkeyManager.hotkey() == 0)
	{
		QMessageBox msgBox(QMessageBox::Critical, "Error",
						   "Hotkey is invalid, please set it. Press Ok to open"\
						   "the settings, or press Cancel to quit albert.",
						   QMessageBox::Close|QMessageBox::Ok);
		msgBox.exec();
		if ( msgBox.result() == QMessageBox::Ok )
			_settingsDialog->show();
		else
			exit(0);
	}



	/* SETUP SIGNAL FLOW */

	// A change in text triggers requests
	connect(_inputLine, SIGNAL(textChanged(QString)), _engine, SLOT(query(QString)));

	// Proposallistview tells Inputline to change text on completion.
	connect(_proposalListView, SIGNAL(completion(QString)), _inputLine, SLOT(setText(QString)));

	// Bottonpress or shortcuts op settings dialog, and close albert once
	connect(_inputLine, SIGNAL(settingsDialogRequested()), this, SLOT(hide()));
	connect(_inputLine, SIGNAL(settingsDialogRequested()), _settingsDialog, SLOT(show()));

	// Show mainwidget if hotkey is pressed
	connect(&_hotkeyManager, SIGNAL(hotKeyPressed()), this, SLOT(toggleVisibility()));



	/* THEME */

	QStringList themeDirs = QStandardPaths::locateAll(
				QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);
	QFileInfoList themes;
	for (QDir d : themeDirs)
		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);

	// Find and apply the theme
	bool success = false;
	for (QFileInfo fi : themes){
		if (fi.baseName() == _theme) {
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
		qFatal("FATAL: Stylefile not found: %s", _theme.toStdString().c_str());
		exit(EXIT_FAILURE);
	}
}

/**************************************************************************/
MainWidget::~MainWidget()
{
	/* SERIALIZE DATA */
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
	QFile f(path);
	if (f.open(QIODevice::ReadWrite| QIODevice::Text)){
		qDebug() << "Serializing to " << path;
		QDataStream out( &f );
		_engine->serilizeData(out);
		_inputLine->serilizeData(out);
		f.close();
	}
	else
		qFatal("FATAL: Could not write to %s", path.toStdString().c_str());


	/* SAVE SETTINGS */
	QSettings settings(QSettings::UserScope, "albert", "albert");

	_engine->saveSettings(settings);
	_proposalListView->saveSettings(settings);
	_inputLine->saveSettings(settings);

	settings.setValue("showCentered", _showCentered);
	settings.setValue("hotkey", QKeySequence(_hotkeyManager.hotkey()).toString());
	settings.setValue("Theme", _theme);

	delete _engine;
}



/*****************************************************************************/
/********************************* S L O T S *********************************/
/**************************************************************************/
void MainWidget::show()
{
	_inputLine->reset();
	QWidget::show();
	if (_showCentered)
		this->move(QApplication::desktop()->screenGeometry().center()
				   -QPoint(rect().right()/2,192 ));
	this->raise();
	this->activateWindow();
	_inputLine->setFocus();
}

/**************************************************************************/
void MainWidget::toggleVisibility()
{
	this->isVisible() ? this->hide() : this->show();
}

/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/**************************************************************************/


#ifdef Q_OS_LINUX
#include "xcb/xcb.h"
#endif
/**************************************************************************//**
 * @brief MainWidget::nativeEvent
 *
 * The purpose of this function is to hide in special casesonly.
 */
bool MainWidget::nativeEvent(const QByteArray &eventType, void *message, long *)
{
#ifdef Q_OS_LINUX
	if (eventType == "xcb_generic_event_t")
	{
		xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
		switch (event->response_type & 127)
		{
		case XCB_FOCUS_OUT: {
			xcb_focus_out_event_t *fe = (xcb_focus_out_event_t *)event;
//			std::cout << "MainWidget::nativeEvent::XCB_FOCUS_OUT\t";
//			switch (fe->mode) {
//			case XCB_NOTIFY_MODE_NORMAL: std::cout << "XCB_NOTIFY_MODE_NORMAL";break;
//			case XCB_NOTIFY_MODE_GRAB: std::cout << "XCB_NOTIFY_MODE_GRAB";break;
//			case XCB_NOTIFY_MODE_UNGRAB: std::cout << "XCB_NOTIFY_MODE_UNGRAB";break;
//			case XCB_NOTIFY_MODE_WHILE_GRABBED: std::cout << "XCB_NOTIFY_MODE_WHILE_GRABBED";break;
//			}
//			std::cout << "\t";
//			switch (fe->detail) {
//			case XCB_NOTIFY_DETAIL_ANCESTOR: std::cout << "ANCESTOR";break;
//			case XCB_NOTIFY_DETAIL_INFERIOR: std::cout << "INFERIOR";break;
//			case XCB_NOTIFY_DETAIL_NONE: std::cout << "NONE";break;
//			case XCB_NOTIFY_DETAIL_NONLINEAR: std::cout << "NONLINEAR";break;
//			case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL: std::cout << "NONLINEAR_VIRTUAL";break;
//			case XCB_NOTIFY_DETAIL_POINTER: std::cout << "POINTER";break;break;
//			case XCB_NOTIFY_DETAIL_POINTER_ROOT: std::cout << "POINTER_ROOT";
//			case XCB_NOTIFY_DETAIL_VIRTUAL: std::cout << "VIRTUAL";break;
//			}
//			std::cout << std::endl;
			if (((fe->mode==XCB_NOTIFY_MODE_GRAB && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR)
					|| (fe->mode==XCB_NOTIFY_MODE_NORMAL && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR ))
					&& !_settingsDialog->isVisible())
				hide();
			break;
		}
		}
	}
#endif
	return false;
}
