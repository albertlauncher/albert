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

#include <algorithm>
#include "xcb/xcb.h"
#include "albert.h"
#include "albertengine.h"
#include "resultwidget.h"
#include "xhotkeymanager.h"

// remove
#include <iostream>

/**************************************************************************//**
 * @brief AlbertWidget::AlbertWidget
 * @param parent
 */
AlbertWidget::AlbertWidget(QWidget *parent)
	: QWidget(parent)
{
	_selectedResultIndex = -1;
	_nItemsToShow = 5;
	_firstVisibleItemIndex=0;

	// Window properties
	setObjectName("albert");
	setWindowTitle("Albert");
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
	_frame3->setObjectName("frame3");
	_frame3->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
	l3->addWidget(_frame3,0,0);

	// Layer 2
	QVBoxLayout *l2 = new QVBoxLayout;
	l2->setMargin(0);
	_frame3->setLayout(l2);

	_frame2 = new QFrame;
	_frame2->setObjectName("frame2");
	_frame2->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	l2->addWidget(_frame2,0,0);

	// Layer 1
	QVBoxLayout *l1 = new QVBoxLayout;
	l1->setMargin(0);
	_frame2->setLayout(l1);

	_frame1 = new QFrame;
	_frame1->setObjectName("frame1");
	_frame1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	l1->addWidget(_frame1,0,0);

	QVBoxLayout *contentLayout = new QVBoxLayout();
	contentLayout->setMargin(0);
	_frame1->setLayout(contentLayout);

	/* Interface */
	_inputLine = new QLineEdit;
	_inputLine->setObjectName("inputline");
	_inputLine->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	contentLayout->addWidget(_inputLine);

	_resultsLayout = new QVBoxLayout();
	_resultsLayout->setMargin(0);
	_resultsLayout->setSpacing(0);
	contentLayout->addLayout(_resultsLayout);

	//Set focus proxies
	this->setFocusProxy(_inputLine);
	_frame1->setFocusProxy(_inputLine);
	_frame2->setFocusProxy(_inputLine);
	_frame3->setFocusProxy(_inputLine);
	this->setFocusPolicy(Qt::StrongFocus);

	// Position
	this->move(QApplication::desktop()->screenGeometry().center() - rect().center());

	// installEventFilter to check if app lost focus
	QApplication::instance()->installEventFilter(this);

	// Show albert if hotkey was pressed
	connect(XHotKeyManager::getInstance(), SIGNAL(hotKeyPressed()), this, SLOT(onHotKeyPressed()), Qt::QueuedConnection);
	// Start listening for the hotkey(s)
	XHotKeyManager::getInstance()->start();

	// React on confirmation in commandline
	connect(_inputLine, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(_inputLine, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));

}

/**************************************************************************//**
 * @brief AlbertWidget::~AlbertWidget
 */
AlbertWidget::~AlbertWidget()
{
	clearResults();
}

/**************************************************************************//**
 * @brief AlbertWidget::hideAndClear
 */
void AlbertWidget::hideAndClear()
{
	QWidget::hide();
	_inputLine->clear();
	clearResults();
}

/**************************************************************************//**
 * @brief AlbertWidget::clear
 */
void AlbertWidget::clearResults()
{
	QLayoutItem *child;
	while ((child = _resultsLayout->takeAt(0)) != 0)  {
		delete child->widget();
		delete child;
	}
}

/**************************************************************************//**
 * @brief AlbertWidget::drawResults
 */
void AlbertWidget::drawResults()
{
	int begin;
	if (_selectedResultIndex <= (_nItemsToShow-1)/2 ) {
		begin=0;
	} else if (_selectedResultIndex > (int)_results.size()-1-_nItemsToShow+(_nItemsToShow-1)/2) {
		begin=_results.size()-_nItemsToShow;
	} else {
		begin=_selectedResultIndex-(_nItemsToShow-1)/2;
	}

	setUpdatesEnabled(false);
	clearResults();
	for (int i = 0; i < _nItemsToShow && i+begin < (int)_results.size(); ++i) {
		// Create a new widget
		ResultWidget *w = new ResultWidget;
		w->setTitle(_results[begin+i]->name());
		w->setAuxInfo(_results[begin+i]->path());
		w->setObjectName((begin+i == _selectedResultIndex)?"selectedResultWidget":"resultWidget");
		_resultsLayout->addWidget(w);
	}
	std::cout <<_resultsLayout->count()<< "/"<< (int)_results.size()<< _resultsLayout->count() << std::endl;
	setUpdatesEnabled(true);
}

/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/**************************************************************************//**
 * @brief AlbertWidget::onHotKeyPressed
 */
void AlbertWidget::onHotKeyPressed()
{
	if (this->isVisible()){
		this->hideAndClear();
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
	std::cout << "_resultsLayout->count(): " << _resultsLayout->count() << std::endl;
	if (!text.isEmpty())
	{
		AlbertEngine::instance()->request(text, _results);
		_selectedResultIndex = 0;
		std::cout << "_results.size(): " <<  _results.size() << std::endl;
		drawResults();
		return;
	}
	clearResults();

}


/**************************************************************************//**
 * @brief AlbertWidget::onReturnPressed
 */
void AlbertWidget::onReturnPressed()
{
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
		this->hideAndClear();
		break;
	case Qt::Key_Up:
		if (_selectedResultIndex != 0) {
			--_selectedResultIndex;
			drawResults();
		}
		std::cout << "UpArrow pressed." << std::endl;
		break;
	case Qt::Key_Down:
		if (_selectedResultIndex != (int)_results.size()-1) {
			++_selectedResultIndex;
			drawResults();
		}
		std::cout << "DownArrow pressed." << std::endl;
		break;
	case Qt::Key_PageUp:
		if (_selectedResultIndex != 0) {
			_selectedResultIndex = std::max(0, _selectedResultIndex-_nItemsToShow);
			drawResults();
		}
		std::cout << "Key_PageUp pressed." << std::endl;
		break;
	case Qt::Key_PageDown:
		if (_selectedResultIndex != (int)_results.size()-1) {
			_selectedResultIndex = std::min(_selectedResultIndex+_nItemsToShow, (int)_results.size()-1);
			drawResults();
		}
		std::cout << "Key_PageDown pressed." << std::endl;
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
	if (event->type() == QEvent::ApplicationStateChange && this->isActiveWindow()) {
		this->hideAndClear();
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
				std::cout << "Ignored XCB_FOCUS_IN event" << std::endl;
				return true; // Ignore this events
			}
			std::cout << "Ignored XCB_FOCUS_IN event NOT" << std::endl;
			break;
		}
		case XCB_FOCUS_OUT: {
			xcb_focus_out_event_t *fe = (xcb_focus_out_event_t *)event;
			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_WHILE_GRABBED|XCB_NOTIFY_MODE_UNGRAB)){
				std::cout << "Ignored XCB_FOCUS_OUT event" << std::endl;
				return true; // Ignore this events
			}
			std::cout << "Ignored XCB_FOCUS_OUT event NOT" << std::endl;
			break;
		}
		}
	}
	return false;
}
