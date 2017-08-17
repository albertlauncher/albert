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
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QQmlContext>
#include <QQuickItem>
#include "mainwindow.h"

const QString MainWindow::CFG_CENTERED        = "showCentered";
const bool    MainWindow::DEF_CENTERED        = true;
const QString MainWindow::CFG_HIDEONFOCUSLOSS = "hideOnFocusLoss";
const bool    MainWindow::DEF_HIDEONFOCUSLOSS = true;
const QString MainWindow::CFG_ALWAYS_ON_TOP   = "alwaysOnTop";
const bool    MainWindow::DEF_ALWAYS_ON_TOP   = true;
const QString MainWindow::CFG_STYLEPATH       = "stylePath";
const QUrl    MainWindow::DEF_STYLEPATH       = QUrl("qrc:/resources/MainComponent.qml");
const QString MainWindow::CFG_WND_POS         = "windowPosition";

/** ***************************************************************************/
MainWindow::MainWindow(QWindow *parent) : QQuickView(parent) {
    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    // Set qml environment
    rootContext()->setContextProperty("history", &history_);
    rootContext()->setContextProperty("resultsModel", &model_);

    // Load settings
    QSettings s;
    setPosition(s.value(CFG_WND_POS).toPoint());
    setShowCentered(s.value(CFG_CENTERED, DEF_CENTERED).toBool());
    setHideOnFocusLoss(s.value(CFG_HIDEONFOCUSLOSS, DEF_HIDEONFOCUSLOSS).toBool());
    setAlwaysOnTop(s.value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setSource(s.value(CFG_STYLEPATH, DEF_STYLEPATH).toUrl());
}


/** ***************************************************************************/
MainWindow::~MainWindow() {
    // Save settings
    QSettings s;
    s.setValue(CFG_CENTERED, showCentered_);
    s.setValue(CFG_HIDEONFOCUSLOSS, hideOnFocusLoss_);
    s.setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop());
    s.setValue(CFG_STYLEPATH, source());
    s.setValue(CFG_WND_POS, position());
    setSource(QUrl()); // Saves the themeconfig

    qDebug() << "QML Box Model mainwindow destructor called";
}


/** ***************************************************************************/
void MainWindow::setVisible(bool visible) {
    if ( visible ) {
        if ( showCentered_ ){
            QDesktopWidget *dw = QApplication::desktop();
            setPosition(dw->availableGeometry(dw->screenNumber(QCursor::pos()))
                        .center()-QPoint(width()/2,192));
        }
        QQuickView::show();
        raise();
        requestActivate();
    } else {
        QQuickView::hide();
        QMetaObject::invokeMethod(rootObject(), "onMainWindowHidden");
    }
}


/** ***************************************************************************/
QString MainWindow::input() {
    QString retVal;
    QMetaObject::invokeMethod(rootObject(), "getInput", Qt::DirectConnection,
                              Q_RETURN_ARG(QString, retVal));
    return retVal;

}


/** ***************************************************************************/
void MainWindow::setInput(const QString &input) {
    QMetaObject::invokeMethod(rootObject(), "setInput", Qt::DirectConnection,
                              Q_ARG(QString, input));
}


/** ***************************************************************************/
void MainWindow::setSource(const QUrl &url) {

    // Prepare the theme property files
    QString p = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
            .filePath("styles.conf");
    QSettings themeProperties(p, QSettings::IniFormat);

    // If source is not empty save all propeties
    if (!source().isEmpty()) {
        themeProperties.beginGroup(source().toEncoded());
        for (QString &prop : availableProperties()) {
            QVariant value = property(prop.toLatin1().data());
            if (value.isValid())
                themeProperties.setValue(prop, value);
        }
        themeProperties.endGroup();
    }

    if (!url.isEmpty()) {

        // Apply the source
        QQuickView::setSource(url);

        // Connect the sigals
        QObject *object = rootObject();
        connect(object, SIGNAL(inputChanged(QString)),
                this, SIGNAL(inputChanged(QString)));
        connect(object, SIGNAL(settingsWidgetRequested()),
                this, SIGNAL(settingsWidgetRequested()));
        connect(this, &MainWindow::settingsWidgetRequested,
                [](){ qDebug() << "HOden";});

        // Load the theme properties
        themeProperties.beginGroup(source().toEncoded());
        for (QString &prop : availableProperties()) {
            if (themeProperties.contains(prop))
                setProperty(prop.toLatin1().data(), themeProperties.value(prop));
        }
        themeProperties.endGroup();
    }
}



/** ***************************************************************************/
QStringList MainWindow::availableProperties() {
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availableProperties",
                              Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}



/** ***************************************************************************/
QVariant MainWindow::property(const char *name) const {
    return rootObject()->property(name);
}



/** ***************************************************************************/
void MainWindow::setProperty(const char *attribute, const QVariant &value) {
    rootObject()->setProperty(attribute, value);
    rootObject()->update();
}



/** ***************************************************************************/
QStringList MainWindow::availablePresets() {
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availablePresets",
                              Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}



/** ***************************************************************************/
void MainWindow::setPreset(const QString &name){
    QMetaObject::invokeMethod(rootObject(), "setPreset",
                              Q_ARG(QVariant, QVariant::fromValue(name)));
}



/** ***************************************************************************/
void MainWindow::setModel(QAbstractItemModel *model) {
    model_.setSourceModel(model);
}



/** ***************************************************************************/
bool MainWindow::event(QEvent *event) {
    switch (event->type())
    {
    // Quit on Alt+F4
    case QEvent::Close:
            qApp->quit();
        return true;

    // Hide window on escape key
    case QEvent::KeyPress:
        if ( static_cast<QKeyEvent*>(event)->modifiers() == Qt::NoModifier
             && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape ){
            hide();
            return true;
        }
        break;

    case QEvent::FocusOut:
        /* This is a horribly hackish but working solution.

         A triggered key grab on X11 steals the focus of the window for short
         period of time. This may result in the following annoying behaviour:
         When the hotkey is pressed and X11 steals the focus there arises a
         race condition between the hotkey event and the focus out event.
         When the app is visible and the focus out event is delivered the app
         gets hidden. Finally when the hotkey is received the app gets shown
         again although the user intended to hide the app with the hotkey.

         Solutions:
         Although X11 differs between the two focus out events, qt does not.
         One might install a native event filter and use the XCB structs to
         decide which type of event is delivered, but this approach is not
         platform independent (unless designed so explicitely, but its a
         hassle). The behaviour was expected when the app hides on:

         (mode==XCB_NOTIFY_MODE_GRAB && detail==XCB_NOTIFY_DETAIL_NONLINEAR)||
          (mode==XCB_NOTIFY_MODE_NORMAL && detail==XCB_NOTIFY_DETAIL_NONLINEAR)
         (Check Xlib Programming Manual)

         The current, much simpler but less elegant solution is to delay the
         hiding a few milliseconds, so that the hotkey event will always be
         handled first. */
        if (static_cast<QFocusEvent*>(event)->reason() == Qt::ActiveWindowFocusReason
                && hideOnFocusLoss_ && !isActive()){
            QTimer::singleShot(50, this, &MainWindow::hide);
        }
        break;
    default:break;
    }
    return QQuickView::event(event);
}



/** ***************************************************************************/
bool MainWindow::alwaysOnTop() const {
    return flags() & Qt::WindowStaysOnTopHint;
}



/** ***************************************************************************/
void MainWindow::setAlwaysOnTop(bool alwaysOnTop) {
    alwaysOnTop
            ? setFlags(flags() | Qt::WindowStaysOnTopHint)
            : setFlags(flags() & ~Qt::WindowStaysOnTopHint);
    // Flags changed. Update
    QQuickView::hide();
}



/** ***************************************************************************/
bool MainWindow::hideOnFocusLoss() const {
    return hideOnFocusLoss_;
}



/** ***************************************************************************/
void MainWindow::setHideOnFocusLoss(bool hideOnFocusLoss) {
    hideOnFocusLoss_ = hideOnFocusLoss;
}



/** ***************************************************************************/
bool MainWindow::showCentered() const {
    return showCentered_;
}



/** ***************************************************************************/
void MainWindow::setShowCentered(bool showCentered) {
    showCentered_ = showCentered;
}
