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
#include "xhotkeymanager.h"
#include "xcb/xcb.h"

// remove
#include <QEvent>


/**************************************************************************//**
 * @brief AlbertWidget::AlbertWidget
 * @param parent
 */
AlbertWidget::AlbertWidget(QWidget *parent)
	: QWidget(parent)
{
	// Window properties
	setObjectName(QString::fromLocal8Bit("albert"));
	setWindowTitle(QString::fromLocal8Bit("Albert"));
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool
					);

	/* Layout hierarchy */

	// Layer 3
	QVBoxLayout *l3 = new QVBoxLayout;
	l3->setMargin(0);
	l3->setSizeConstraint(QLayout::SetFixedSize);
	this->setLayout(l3);

	_frame3 = new QFrame;
	_frame3->setObjectName(QString::fromLocal8Bit("frame3"));
	_frame3->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
	l3->addWidget(_frame3,0,0);

	// Layer 2
	QVBoxLayout *l2 = new QVBoxLayout;
	l2->setMargin(0);
	_frame3->setLayout(l2);

	_frame2 = new QFrame;
	_frame2->setObjectName(QString::fromLocal8Bit("frame2"));
	_frame2->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	l2->addWidget(_frame2,0,0);

	// Layer 1
	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	_frame2->setLayout(l1);

	_frame1 = new QFrame;
	_frame1->setObjectName(QString::fromLocal8Bit("frame1"));
	_frame1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	l1->addWidget(_frame1,0,0);

	QVBoxLayout *contentLayout = new QVBoxLayout();
	contentLayout->setMargin(0);
	_frame1->setLayout(contentLayout);

	/* Interface */
	_inputLine = new InputLine;
	contentLayout->addWidget(_inputLine);

	_proposalListModel = new ProposalListModel;
	_proposalListView = new ProposalListView;
	_proposalListView->setModel(_proposalListModel);
	_proposalListView->hide();
	contentLayout->addWidget(_proposalListView);

	//Set focus proxies
	this->setFocusProxy(_inputLine);
	_frame1->setFocusProxy(_inputLine);
	_frame2->setFocusProxy(_inputLine);
	_frame3->setFocusProxy(_inputLine);
	_proposalListView->setFocusProxy(_inputLine);
	this->setFocusPolicy(Qt::StrongFocus);

	// Install EventFilter and signals
	QApplication::instance()->installEventFilter(this); // check if app lost focus
	_inputLine->installEventFilter(_proposalListView); // intercept navigation, handle modifiers
	connect(_inputLine, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited(QString)));
	connect(_proposalListView, SIGNAL(completion(QString)), _inputLine, SLOT(onCompletion(QString)));
	connect(XHotKeyManager::getInstance(), SIGNAL(hotKeyPressed()), this, SLOT(onHotKeyPressed()), Qt::QueuedConnection);// Show albert if hotkey was pressed
	XHotKeyManager::getInstance()->start(); // Start listening for the hotkey(s)
}

/**************************************************************************//**
 * @brief AlbertWidget::~AlbertWidget
 */
AlbertWidget::~AlbertWidget()
{
}

/*****************************************************************************/
/********************************* S L O T S *********************************/
/**************************************************************************//**
 * @brief AlbertWidget::hide
 */
void AlbertWidget::hide()
{
	QWidget::hide();
	_inputLine->clear();
	_proposalListModel->clear();
	_proposalListView->hide();
}

/**************************************************************************//**
 * @brief AlbertWidget::show
 */
void AlbertWidget::show()
{
	QWidget::show();
	this->move(QApplication::desktop()->screenGeometry().center() - rect().center() -QPoint(0,256 ));
}

/**************************************************************************//**
 * @brief AlbertWidget::onHotKeyPressed
 */
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

/**************************************************************************//**
 * @brief AlbertWidget::onTextEdited
 * @param text
 */
void AlbertWidget::onTextEdited(const QString & text)
{
	if (!text.isEmpty()){
		const std::vector<AbstractServiceProvider::AbstractItem *> &r = AlbertEngine::instance()->query(text);
		if (!r.empty()) {
			_proposalListModel->set(r);
			_proposalListView->updateGeometry(); // TODO besseren platz finden in
			_proposalListView->show();
			return;
		}
	}
	_proposalListModel->clear();
	_proposalListView->hide();
}

/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/**************************************************************************//**
 * @brief AlbertWidget::keyPressEvent
 * @param event
 */
void AlbertWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Escape:
		this->hide();
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

/**************************************************************************//**
 * @brief AlbertWidget::eventFilter
 * Handle focus loss of the app
 * @param obj
 * @param event
 * @return
 */
bool AlbertWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ApplicationStateChange && this->isActiveWindow())
	{
		this->hide();
		return true;
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
