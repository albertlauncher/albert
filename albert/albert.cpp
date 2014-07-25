#include "albert.h"
#include "xhotkeymanager.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QStandardPaths>

#include "xcb/xcb.h"

// remove

#include <iostream>
#include <QtX11Extras/QX11Info>

/**************************************************************************//**
 * @brief AlbertWidget::AlbertWidget
 * @param parent
 */
AlbertWidget::AlbertWidget(QWidget *parent)
	: QWidget(parent)
{
    // Window properties
    setObjectName("albert");
    setWindowTitle("Albert");
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::ClickFocus);
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool);

	// Layout hierarchy
	 _commandLine = new CommandLine(this);

	QVBoxLayout *verticalLayout = new QVBoxLayout();
	verticalLayout->addWidget(_commandLine);
    verticalLayout->setMargin(0);

    QFrame * topFrame = new QFrame;
    topFrame->setObjectName("topFrame");
	topFrame->setFocusProxy(_commandLine);
    topFrame->setLayout(verticalLayout);

    QVBoxLayout *topLayer = new QVBoxLayout;
    topLayer->addWidget(topFrame,0,0);
    topLayer->setMargin(0);

    QFrame * bottomFrame = new QFrame;
    bottomFrame->setObjectName("bottomFrame");
    bottomFrame->setFocusProxy(topFrame);
    bottomFrame->setLayout(topLayer);

    QVBoxLayout *bottomLayer = new QVBoxLayout;
    bottomLayer->addWidget(bottomFrame,0,0);
    bottomLayer->setMargin(0);

    this->setLayout(bottomLayer);
	this->setFocusProxy(bottomFrame);
	this->setFocusPolicy(Qt::StrongFocus);
    this->adjustSize();

    // Position
    this->move(QApplication::desktop()->screenGeometry().center() - rect().center());

//	// Check if app lost focus
	QApplication::instance()->installEventFilter(this);
	_commandLine->installEventFilter(this);

    // Show albert if hotkey was pressed
	connect(XHotKeyManager::getInstance(), SIGNAL(hotKeyPressed()), this, SLOT(onHotKeyPressed()), Qt::QueuedConnection);

    // Start listening for the hotkey(s)
	XHotKeyManager::getInstance()->start();

	// Build the index
	_engine.buildIndex();



	// tesing area


}

/**************************************************************************//**
 * @brief AlbertWidget::~AlbertWidget
 */
AlbertWidget::~AlbertWidget()
{

}

/**************************************************************************//**
 * @brief AlbertWidget::toggle
 */
void AlbertWidget::toggle()
{
	if (this->isVisible()){
		this->hideAndClear();
		return;
	}
	this->show();
	this->activateWindow();
	this->raise();
	_commandLine->setFocus();
}

/**************************************************************************//**
 * @brief AlbertWidget::hideAndClear
 */
void AlbertWidget::hideAndClear()
{
	QWidget::hide();
	_commandLine->clear();
}

/**************************************************************************//**
 * @brief AlbertWidget::onHotKeyPressed
 */
void AlbertWidget::onHotKeyPressed()
{
	/*
	* TODO MAKE A HACK NOTE
	*/
	std::cout << "Hot key emitted!" << std::endl;

	if (this->isVisible()){
		this->hideAndClear();
		return;
	}
	this->show();
	this->activateWindow();
	this->raise();
	_commandLine->setFocus();
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
	case Qt::Key_Return:
		if (!_commandLine->text().isEmpty())
			QDesktopServices::openUrl(QUrl(QString("https://www.google.de/search?q=%1").arg(_commandLine->text())));
		this->hideAndClear();
		break;
	case Qt::Key_Escape:
		this->hideAndClear();
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

/**************************************************************************//**
 * @brief AlbertWidget::focusOutEvent
 * @param e
 */
void AlbertWidget::focusOutEvent(QFocusEvent* e)
{
//	if (e->reason() != Qt::TabFocusReason
//			||e->reason() != Qt::BacktabFocusReason
//			||e->reason() != Qt::ActiveWindowFocusReason
//			||e->reason() != Qt::PopupFocusReason
//			||e->reason() != Qt::ShortcutFocusReason
//			||e->reason() != Qt::MenuBarFocusReason
//			||e->reason() != Qt::OtherFocusReason)
//		return;
	std::cout << " focusOutEvent" << std::endl;
	this->hideAndClear();
	QWidget::focusOutEvent(e);
}

/**************************************************************************//**
 * @brief AlbertWidget::eventFilter
 * @param obj
 * @param event
 * @return
 */
bool AlbertWidget::eventFilter(QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::ApplicationStateChange && this->isActiveWindow()) {
	   std::cout << "Hidden by  ApplicationStateChange" << std::endl;
	   this->hideAndClear();
	  return true; // The event is handled
   }
//   if (obj == _commandLine) {

////	   this->hideAndClear();
//	   std::cout << " _commandLine" << std::endl;
//	   std::cout << event->type() << std::endl;
//	 // return false; // The event is handled
//   }
   return QObject::eventFilter(obj, event); // Unhandled events are passed to the base class
}

/**************************************************************************//**
 * @brief AlbertWidget::nativeEvent
 * This special event handler can be reimplemented in a subclass to receive
 * native platform events identified by eventType which are passed in the
 * message parameter. In your reimplementation of this function, if you want to
 * stop the event being handled by Qt, return true and set result. If you
 * return false, this native event is passed back to Qt, which translates the
 * event into a Qt event and sends it to the widget.
 *
 * This method is called for every native event. On X11, eventType is set to
 * "xcb_generic_event_t", and the message can be casted to a
 * xcb_generic_event_t pointer.
 *
 * @param eventType
 * @param message
 * @return Indicator if this event shall be stoped being handled further.
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
			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_UNGRAB)){
				return true; // Ignore this events
			}
			break;
		}
		case XCB_FOCUS_OUT: {
			xcb_focus_out_event_t *fe = (xcb_focus_out_event_t *)event;
			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_UNGRAB)){
				return true; // Ignore this events
			}
			break;
		}
		}
	}
	return false;
}


				//		if (event->response_type & ~0x80  & (XCB_FOCUS_IN|XCB_FOCUS_OUT))
//		{
//			if (event->response_type & ~0x80 & (XCB_FOCUS_IN))
//				std::cout << "eventType.XCB_FOCUS_IN()" << std::endl;
//			if (event->response_type & ~0x80 & (XCB_FOCUS_OUT))
//				std::cout << "eventType.XCB_FOCUS_OUT()" << std::endl;
//			// xcb_focus_out_event_t is a typedef of xcb_focus_in_event_t
//			// and hence can be handled equally
//			xcb_focus_in_event_t *fe = (xcb_focus_in_event_t *)event;
//			std::cout << "" << fe->response_type << std::endl;
//			std::cout << "" << fe->detail << std::endl;
//			std::cout << "" << fe->detail << std::endl;
//			std::cout << "" << fe->detail << std::endl;
//			std::cout << "" << fe->detail << std::endl;
//			std::cout << "" << fe->detail << std::endl;
//			// If this is a focus event generated by a grab ignore it
//			if (fe->mode & (XCB_NOTIFY_MODE_GRAB|XCB_NOTIFY_MODE_UNGRAB))
//				return false; // stop being handled further
//		}
//	}
//	return false;
//}










//    TRASH     //

//    QGraphicsBlurEffect* dropShadowEffect = new QGraphicsBlurEffect(this);
//    dropShadowEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
//    dropShadowEffect->setBlurRadius(5);
//    this->setGraphicsEffect(dropShadowEffect);
