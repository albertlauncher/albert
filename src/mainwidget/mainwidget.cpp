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

/****************************************************************************///
MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	// INITIALIZE UI
    ui.setupUi(this);
    setWindowTitle(qAppName());
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Tool
                   | Qt::WindowStaysOnTopHint
                   | Qt::WindowCloseButtonHint // No close event w/o this
                   | Qt::FramelessWindowHint);

    ui.bottomLayout->setSizeConstraint(QLayout::SetFixedSize);

    ui.bottomLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    ui.topLayout->setAlignment    (Qt::AlignHCenter | Qt::AlignTop);
    ui.contentLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    ui.bottomFrame->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui.topFrame->setSizePolicy    (QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui.inputLine->setSizePolicy   (QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui.proposalList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Do not allow context menues (they cause focus out events)
    ui.inputLine->setContextMenuPolicy(Qt::NoContextMenu);
    ui.proposalList->setContextMenuPolicy(Qt::NoContextMenu);

    // Let proposalList not accept keyboard focus
    ui.proposalList->setFocusPolicy(Qt::NoFocus);

    // Let inputLine get focus when proposallist gets it.
    ui.proposalList->setFocusProxy(ui.inputLine);

    // Proposallistview intercepts inputline's events (Navigation with keys, pressed modifiers, etc)
    ui.inputLine->installEventFilter(ui.proposalList);

    ui.proposalList->hide();

    // Settings
    QSettings s;
    _showCentered = s.value(CFG_CENTERED, CFG_CENTERED_DEF).toBool();
    _theme = s.value(CFG_THEME, CFG_THEME_DEF).toString();
    if (!setTheme(_theme)){
        qFatal("FATAL: Stylefile not found: %s", _theme.toStdString().c_str());
        exit(EXIT_FAILURE);
    }
}

/****************************************************************************///
MainWidget::~MainWidget()
{
    // Save settings
    QSettings s;
    s.setValue(CFG_CENTERED, _showCentered);
    s.setValue(CFG_THEME, _theme);
}


/*****************************************************************************/
/********************************* S L O T S *********************************/
/****************************************************************************///
void MainWidget::show()
{
    ui.inputLine->clear();
    QWidget::show();
    if (_showCentered)
        this->move(QApplication::desktop()->screenGeometry().center()
                   -QPoint(rect().right()/2,192 ));
    this->raise();
    this->activateWindow();
    ui.inputLine->setFocus();
    emit widgetShown();
}
/****************************************************************************///
void MainWidget::hide()
{
    QWidget::hide();
    emit widgetHidden();
}



/** ***************************************************************************/
void MainWidget::toggleVisibility() {
    this->isVisible() ? this->hide() : this->show();
}



/** ***************************************************************************/
void MainWidget::setShowCentered(bool b) {
    _showCentered = b;
}



/** ***************************************************************************/
bool MainWidget::showCenterd() const {
    return _showCentered;
}



/** ***************************************************************************/
const QString &MainWidget::theme() const {
    return _theme;
}



/** ***************************************************************************/
bool MainWidget::setTheme(const QString &theme) {
    _theme = theme;
    QFileInfoList themes;
    QStringList themeDirs = QStandardPaths::locateAll(
        QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);
    for (QDir d : themeDirs)
        themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
    // Find and apply the theme
    bool success = false;
    for (QFileInfo fi : themes){
        if (fi.baseName() == _theme) {
            QFile f(fi.canonicalFilePath());
            if (f.open(QFile::ReadOnly)) {
                qApp->setStyleSheet(f.readAll());
                f.close();
                success = true;
                break;
            }
        }
    }
    return success;
}


/*****************************************************************************/
/**************************** O V E R R I D E S ******************************/
/****************************************************************************///

/** ***************************************************************************/
void MainWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
    qApp->quit();
}

/** ***************************************************************************/
void MainWidget::keyPressEvent(QKeyEvent *e)
{
    // Hide window on escape key
    if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Escape ) {
        hide();
        e->accept();
    }
    QWidget::keyPressEvent(e);
}

#ifdef Q_OS_LINUX
#include "xcb/xcb.h"
/** ****************************************************************************
 * @brief MainWidget::nativeEvent
 *
 * The purpose of this function is to hide in special casesonly.
 */
bool MainWidget::nativeEvent(const QByteArray &eventType, void *message, long *)
{
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
    return false;
}
#endif
