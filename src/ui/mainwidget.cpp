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
#include "globalhotkey.h"
#include <QEvent>
#include <QLabel>
#include <QFile>
#include <QStandardPaths>
#include "globals.h"

/**************************************************************************/
MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	/* MISC */

	_engine = new Engine;
	deserialize();
	connect(GlobalHotkey::instance(), SIGNAL(hotKeyPressed()), this, SLOT(toggleVisibility()));
	GlobalHotkey::instance()->setHotkey({Qt::AltModifier, Qt::Key_Space});

	/* UI and windowing */

	// Window properties
	setObjectName(QString::fromLocal8Bit("albert"));
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
	this->setLayout(l2);

	// Layer 2

	_frame2 = new QFrame;
	_frame2->setObjectName(QString::fromLocal8Bit("bottomframe"));
	_frame2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	l2->addWidget(_frame2,0,0);

	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	_frame2->setLayout(l1);

	// Layer 1

	_frame1 = new QFrame;
	_frame1->setObjectName(QString::fromLocal8Bit("topframe"));
	_frame1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	l1->addWidget(_frame1,0,0);

	QVBoxLayout *contentLayout = new QVBoxLayout();
	contentLayout->setMargin(0);
	_frame1->setLayout(contentLayout);

	// ContentLayer

	_inputLine = new InputLine;
	// A change in text triggers requests
	connect(_inputLine, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited(QString)));
	contentLayout->addWidget(_inputLine);

	_proposalListView = new ProposalListView;
	_proposalListView->setFocusPolicy(Qt::NoFocus);
	_proposalListView->setFocusProxy(_inputLine);
	_proposalListView->hide();
	_proposalListView->setModel(_engine);
	// Proposallistview tells Inputline to change text (completion)
	connect(_proposalListView, SIGNAL(completion(QString)), _inputLine, SLOT(setText(QString)));
	// Proposallistview intercepts inputline's events (Navigation with keys, pressed modifiers, etc)
	_inputLine->installEventFilter(_proposalListView);
	contentLayout->addWidget(_proposalListView);
}

/**************************************************************************/
MainWidget::~MainWidget()
{
	serialize();
}

/**************************************************************************/
void MainWidget::serialize() const
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
	QFile f(path);
	if (f.open(QIODevice::ReadWrite| QIODevice::Text)){
		qDebug() << "Serializing to " << path;
		QDataStream out( &f );
		_engine->serialize(out);
		f.close();
		return;
	}
	qFatal("FATAL: Could not write to %s", path.toStdString().c_str());
}

/**************************************************************************/
void MainWidget::deserialize()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db";
	QFile f(path);
	if (f.open(QIODevice::ReadOnly| QIODevice::Text))
	{
		qDebug() << "Deserializing from" << path;
		QDataStream in( &f );
		_engine->deserialize(in);
		f.close();
	}
	else
	{
		qWarning() << "Could not open file" << path;
		_engine->initialize();
	}

}

/*****************************************************************************/
/********************************* S L O T S *********************************/
/**************************************************************************/
void MainWidget::show()
{
	_engine->clear();
	_proposalListView->hide();
	QWidget::show();
	_inputLine->clear();
	updateGeometry();
	if (gSettings->value(QString::fromLocal8Bit("showCentered"), QString::fromLocal8Bit("true")).toBool())
		this->move(QApplication::desktop()->screenGeometry().center() - QPoint(rect().right()/2,192 ));
	this->raise();
	this->activateWindow();
	_inputLine->setFocus();
}

/**************************************************************************/
void MainWidget::toggleVisibility()
{
	this->isVisible() ? this->hide() : this->show();
}

/**************************************************************************/
void MainWidget::onTextEdited(const QString & text)
{
	QString t = text.trimmed();
	if (!t.isEmpty()){
		_engine->query(t);
		if (_engine->rowCount() > 0){
			if (!_proposalListView->currentIndex().isValid())
				_proposalListView->setCurrentIndex(_engine->index(0, 0));
		}
		_proposalListView->show();
		return;
	}
	_engine->clear();
	_proposalListView->hide();
}

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
			if ((fe->mode==XCB_NOTIFY_MODE_GRAB && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR)
					|| (fe->mode==XCB_NOTIFY_MODE_NORMAL && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR ))
				hide();
			break;
		}
		}
	}
#endif
return false;
}
