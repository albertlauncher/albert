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

#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopWidget>
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include "mainwidget.h"
#include "settings.h"

/****************************************************************************///
MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	// INITIALIZE UI
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

	_proposalListView = new ProposalListView;
	_proposalListView->setObjectName("proposallist");
	_proposalListView->setFocusPolicy(Qt::NoFocus);
	_proposalListView->setFocusProxy(_inputLine);
	_proposalListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//	_proposalListView->hide();
	contentLayout->addWidget(_proposalListView);
	// _proposalListView->setModel(_engine);
	// Proposallistview intercepts inputline's events (Navigation with keys, pressed modifiers, etc)
	_inputLine->installEventFilter(_proposalListView);


	// LOAD SETTINGS
	QSettings s(QSettings::UserScope, "albert", "albert");
//	_proposalListView->loadSettings(s);
    _inputLine->loadSettings(s);


	// SETUP SIGNAL FLOW
	// Proposallistview tells Inputline to change text on completion.
//	connect(_proposalListView, &ProposalListView::completion,
//			_inputLine, &QLineEdit::setText);


}

/****************************************************************************///
MainWidget::~MainWidget()
{
	/*
	 *  SAVE SETTINGS
	 */

//	QSettings settings(QSettings::UserScope, "albert", "albert");

//	_engine->saveSettings(settings);
//	_proposalListView->saveSettings(settings);
//	_inputLine->saveSettings(settings);
}

/*****************************************************************************/
/********************************* S L O T S *********************************/
/****************************************************************************///
void MainWidget::show()
{
	_inputLine->reset();
	QWidget::show();
    if (gSettings->value(CFG_CENTERED, CFG_CENTERED_DEF).toBool())
		this->move(QApplication::desktop()->screenGeometry().center()
				   -QPoint(rect().right()/2,192 ));
	this->raise();
	this->activateWindow();
	_inputLine->setFocus();
	emit widgetShown();
}
/****************************************************************************///
void MainWidget::hide()
{
	QWidget::hide();
	emit widgetHidden();
}

/****************************************************************************///
void MainWidget::toggleVisibility()
{
	this->isVisible() ? this->hide() : this->show();
}

/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/****************************************************************************///


#ifdef Q_OS_LINUX
#include "xcb/xcb.h"
#endif
/***************************************************************************//**
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
					|| (fe->mode==XCB_NOTIFY_MODE_NORMAL && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR )))
//					&& !_settingsDialog->isVisible())
				hide();
			break;
		}
		}
	}
#endif
	return false;
}



/////////////////////////////////////////////TRASH//////////////////////////////

//	QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db");
//	if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
//		qDebug() << "Deserializing from" << f.fileName();
//		QDataStream in(&f);
//		_engine->deserilizeData(in);
//		_inputLine->deserilizeData(in);
//		f.close();
//	} else {
//		qWarning() << "Could not open file" << f.fileName();
//		_engine->initialize();
//	}

//	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
//	QFile f(path);
//	if (f.open(QIODevice::ReadWrite| QIODevice::Text)){
//		qDebug() << "Serializing to " << path;
//		QDataStream out( &f );
//		_engine->serilizeData(out);
//		_inputLine->serilizeData(out);
//		f.close();
//	}
//	else
//		qFatal("FATAL: Could not write to %s", path.toStdString().c_str());
