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

#include "albert.h"
#include "albertengine.h"
#include "settingsdialog.h"
#include "xhotkeymanager.h"
#include "xcb/xcb.h"
#include <QEvent>
#include <QSettings>
#include <QLabel>

/**************************************************************************/
AlbertWidget::AlbertWidget(QWidget *parent)
	: QWidget(parent)
{
	/* Stuff concerning the UI and windowing */


	// Window properties
	setObjectName(QString::fromLocal8Bit("albert"));
	setWindowTitle(QString::fromLocal8Bit("Albert"));
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool
					);

	// Layer 2
	QVBoxLayout *l2 = new QVBoxLayout;
	l2->setMargin(0);
	l2->setSizeConstraint(QLayout::SetFixedSize);
	this->setLayout(l2);

	_frame2 = new QFrame;
	_frame2->setObjectName(QString::fromLocal8Bit("bottomframe"));
	_frame2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	l2->addWidget(_frame2,0,0);

	// Layer 1
	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	_frame2->setLayout(l1);

	_frame1 = new QFrame;
	_frame1->setObjectName(QString::fromLocal8Bit("topframe"));
	_frame1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	l1->addWidget(_frame1,0,0);


	QVBoxLayout *contentLayout = new QVBoxLayout();

	// Interface
	_inputLine = new InputLine;
	contentLayout->addWidget(_inputLine);
	_proposalListView = new ProposalListView;
	_proposalListView->setModel(&_engine);
	_proposalListView->hide();
	contentLayout->addWidget(_proposalListView);
	contentLayout->setMargin(0);
	_frame1->setLayout(contentLayout);

	//Set focus proxies
	this->setFocusProxy(_inputLine);
	_frame1->setFocusProxy(_inputLine);
	_frame2->setFocusProxy(_inputLine);
	_proposalListView->setFocusProxy(_inputLine);
	this->setFocusPolicy(Qt::StrongFocus);


	/* Sniffing and snooping*/

	// Albert intercepts inputline (Enter, Tab(Completion) and focus-loss handling)
	_inputLine->installEventFilter(this);

	// Listview intercepts inputline (Navigation with keys, pressed modifiers)
	_inputLine->installEventFilter(_proposalListView);

	// A change in text triggers requests
	connect(_inputLine, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited(QString)));

	connect(XHotKeyManager::getInstance(), SIGNAL(hotKeyPressed()), this, SLOT(onHotKeyPressed()), Qt::QueuedConnection);// Show albert if hotkey was pressed
	XHotKeyManager::getInstance()->start(); // Start listening for the hotkey(s)

}

/**************************************************************************/
AlbertWidget::~AlbertWidget()
{
}

/*****************************************************************************/
/********************************* S L O T S *********************************/
/**************************************************************************/
void AlbertWidget::hide()
{
	QWidget::hide();
	_inputLine->clear();
	_engine.clear();
	_proposalListView->hide();
}

/**************************************************************************/
void AlbertWidget::show()
{
	QWidget::show();
	updateGeometry();
	if (QSettings().value(QString::fromLocal8Bit("show_centered"), QString::fromLocal8Bit("true")).toBool())
		this->move(QApplication::desktop()->screenGeometry().center() - QPoint(rect().right()/2,192 ));
}

/**************************************************************************/
void AlbertWidget::onHotKeyPressed()
{
	if (this->isVisible()){
		this->hide();
		return;
	}
	this->show();
	this->raise();
	this->activateWindow();
	_inputLine->setFocus();
}

/**************************************************************************/
void AlbertWidget::onTextEdited(const QString & text)
{
	QString t = text.trimmed();
	if (!t.isEmpty()){
		_engine.query(t);
		if (_engine.rowCount() > 0){
			if (!_proposalListView->currentIndex().isValid())
				_proposalListView->setCurrentIndex(_engine.index(0, 0));
		}
		_proposalListView->show();
		return;
	}
	_engine.clear();
	_proposalListView->hide();
}

/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/**************************************************************************/
bool AlbertWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusOut)
	{
		qDebug() << "QEvent::FocusOut";
		this->hide();
		return true;
	}
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		switch (keyEvent->key()) {
		case Qt::Key_Tab:
			// Completion
			// For the definition of the Userroles see proposallistmodel.cpp (::data())
			if (_proposalListView->currentIndex().isValid())
				_inputLine->setText(_engine.data(_proposalListView->currentIndex(), Qt::UserRole+4).toString());
			return true;
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			// Confirmation
			if (!_proposalListView->currentIndex().isValid())
				return true;

			switch (keyEvent->modifiers()) {
			case Qt::ControlModifier:
				_engine.ctrlAction(_proposalListView->currentIndex());
				break;
			case Qt::AltModifier:
				_engine.altAction(_proposalListView->currentIndex());
				break;
			case Qt::NoModifier:
				_engine.action(_proposalListView->currentIndex());
				break;
			default:
				break;
			}
			this->hide();
			return true;
			break;
		case Qt::Key_Comma:
			SettingsDialog::instance()->show();
			return true;
		case Qt::Key_F4:
			qDebug() << "quit.";
			qApp->quit();
			return true;
		case Qt::Key_Escape:
			this->hide();
			return true;
		}
	}
	return QObject::eventFilter(obj, event); // Unhandled events are passed to the base class
}

/**************************************************************************//**
 * @brief AlbertWidget::nativeEvent
 *
 * This special event handler can be reimplemented in a subclass to receive
 * native platform events identified by eventType which are passed in the
 * message parameter.
 * This method is called for every native event. On X11, eventType is set to
 * "xcb_generic_event_t", and the message can be casted to a
 * xcb_generic_event_t pointer.
 * The purpose of this function is to eat malicious focus events generated by
 * X11 when the keyboard is grabbed when the hotkey is pressed.
 *
 * @param eventType
 * @param message
 * @return Indicator if this event shall be stopped being handled by Qt.
 */
bool AlbertWidget::nativeEvent(const QByteArray &eventType, void *message, long *)
{
	if (eventType == "xcb_generic_event_t")
	{
		xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
		switch (event->response_type & ~0x80)
		{
		case XCB_FOCUS_IN: {
			xcb_focus_in_event_t *fe = (xcb_focus_in_event_t *)event;
			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_WHILE_GRABBED|XCB_NOTIFY_MODE_UNGRAB)){
				return true; // Ignore this events
			}
			break;
		}
		case XCB_FOCUS_OUT: {
			xcb_focus_out_event_t *fe = (xcb_focus_out_event_t *)event;
			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_WHILE_GRABBED|XCB_NOTIFY_MODE_UNGRAB)){
				return true; // Ignore this events
			}
			break;
		}
		}
	}
	return false;
}
