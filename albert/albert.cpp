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
	setWindowFlags( Qt::CustomizeWindowHint
					| Qt::FramelessWindowHint
					| Qt::WindowStaysOnTopHint
					| Qt::Tool
					);

	/* Layout hierarchy */

	//Bottomlayer
	QVBoxLayout *bottomLayout = new QVBoxLayout;
	bottomLayout->setMargin(0);
	this->setLayout(bottomLayout);

	QFrame * bottomFrame = new QFrame(this);
	bottomFrame->setObjectName("bottomFrame");
	bottomLayout->addWidget(bottomFrame,0,0);

	// toplayer
	QVBoxLayout *topLayout = new QVBoxLayout;
	topLayout->setMargin(0);
	bottomFrame->setLayout(topLayout);

	QFrame * topFrame = new QFrame(bottomFrame);
	topFrame->setObjectName("topFrame");
	topLayout->addWidget(topFrame,0,0);

	// Content
	QVBoxLayout *verticalLayout = new QVBoxLayout();
	verticalLayout->setMargin(0);
	topFrame->setLayout(verticalLayout);

	_commandLine = new CommandLine(topFrame);
	verticalLayout->addWidget(_commandLine);

	//Set focus proxies
	this->setFocusProxy(_commandLine);
	bottomFrame->setFocusProxy(_commandLine);
	topFrame->setFocusProxy(_commandLine);

	this->setFocusPolicy(Qt::StrongFocus);
    this->adjustSize();

    // Position
    this->move(QApplication::desktop()->screenGeometry().center() - rect().center());

	// installEventFilter to check if app lost focus
	QApplication::instance()->installEventFilter(this);

    // Show albert if hotkey was pressed
	connect(XHotKeyManager::getInstance(), SIGNAL(hotKeyPressed()), this, SLOT(onHotKeyPressed()), Qt::QueuedConnection);
	// React on confirmation in commandline
	connect(_commandLine, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));

    // Start listening for the hotkey(s)
	XHotKeyManager::getInstance()->start();

	// Build the index
	_engine.buildIndex();

	// // testing area // //




}

/**************************************************************************//**
 * @brief AlbertWidget::~AlbertWidget
 */
AlbertWidget::~AlbertWidget()
{

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
//	std::cout << ":: HotKeyPressed()" << std::endl;
	if (this->isVisible()){
		this->hideAndClear();
		return;
	}
	this->show();
	this->raise();
	this->activateWindow();
	//_commandLine->setFocus();

	if (isVisible())std::cout << "isVisible"<< std::endl;
	if (isActiveWindow())std::cout << "isActiveWindow"<< std::endl;
	if (hasFocus())std::cout << "hasFocus"<< std::endl;
	if (isTopLevel())std::cout << "isTopLevel"<< std::endl;
	if (isWindowType())std::cout << "isWindowType"<< std::endl;

}

/**************************************************************************//**
 * @brief AlbertWidget::onReturnPressed
 */
void AlbertWidget::onReturnPressed()
{
	if (!_commandLine->text().isEmpty())
		QDesktopServices::openUrl(QUrl(QString("https://www.google.de/search?q=%1").arg(_commandLine->text())));
	this->hideAndClear();
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
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

/**************************************************************************//**
 * @brief AlbertWidget::eventFilter
 *
 * Handle focus loss of the app
 *
 * @param obj
 * @param event
 * @return
 */
bool AlbertWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ApplicationStateChange && this->isActiveWindow()) {
		std::cout << "Hidden by  ApplicationStateChange" << std::endl;
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






//    TRASH     //

//    QGraphicsBlurEffect* dropShadowEffect = new QGraphicsBlurEffect(this);
//    dropShadowEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
//    dropShadowEffect->setBlurRadius(5);
//    this->setGraphicsEffect(dropShadowEffect);
