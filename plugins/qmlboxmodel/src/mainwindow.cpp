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
#include <QDirIterator>
#include <QDesktopWidget>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include "mainwindow.h"
#ifdef __unix__
#include <X11/extensions/shape.h>
#undef KeyPress
#undef KeyRelease
#undef FocusOut
#include <QtX11Extras/QX11Info>
#endif

namespace {
const QString CFG_CENTERED        = "showCentered";
const bool    DEF_CENTERED        = true;
const QString CFG_HIDEONFOCUSLOSS = "hideOnFocusLoss";
const bool    DEF_HIDEONFOCUSLOSS = true;
const QString CFG_ALWAYS_ON_TOP   = "alwaysOnTop";
const bool    DEF_ALWAYS_ON_TOP   = true;
const QString CFG_STYLEPATH       = "stylePath";
const QString CFG_WND_POS         = "windowPosition";
const QString PLUGIN_ID           = "org.albert.frontend.boxmodel.qml";
const QString STYLE_MAIN_NAME     = "MainComponent.qml";
const char*   CFG_HIDE_ON_CLOSE   = "hideOnClose";
const bool    DEF_HIDE_ON_CLOSE   = false;
}

/** ***************************************************************************/
QmlBoxModel::MainWindow::MainWindow(QSettings *settings, QWindow *parent) : QQuickView(parent) {
    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    settings_ = settings;

    // Set qml environment
    rootContext()->setContextProperty("mainWindow", this);
    rootContext()->setContextProperty("history", &history_);
    rootContext()->setContextProperty("resultsModel", &model_);

    connect(engine(), SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));

    QStringList pluginDataPaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                            "org.albert.frontend.boxmodel.qml",
                                                            QStandardPaths::LocateDirectory);

    // Add the shared modules to the lookup path
    for (const QString &pluginDataPath : pluginDataPaths){
        QDir pluginDataDir = QDir(pluginDataPath);
        if ( pluginDataDir.exists("shared") )
            engine()->addImportPath(pluginDataDir.filePath("shared"));
    }

    // Get style files
    QFileInfoList styles;
    for (const QString &pluginDataPath : pluginDataPaths) {
        QDirIterator it(QString("%1/styles").arg(pluginDataPath), QDir::Dirs|QDir::NoDotAndDotDot);
        while ( it.hasNext() ) {
            QDir root = QDir(it.next());
            if ( root.exists(STYLE_MAIN_NAME) ){
                QmlStyleSpec style;
                style.mainComponent = root.filePath(STYLE_MAIN_NAME);
                style.name          = root.dirName();
                style.author        = "N/A";
                style.version       = "N/A";
                if ( root.exists("metadata.json") ) {
                    QFile file(root.filePath("metadata.json"));
                    if (file.open(QIODevice::ReadOnly)) {
                        QJsonObject metadata = QJsonDocument::fromJson(file.readAll()).object();
                        if (metadata.contains("name"))
                            style.name = metadata["name"].toString();
                        if (metadata.contains("author"))
                            style.author = metadata["author"].toString();
                        if (metadata.contains("version"))
                            style.version = metadata["version"].toString();
                    }
                }
                styles_.push_back(style);
            }
        }
    }

    if (styles_.empty())
        throw "No styles found.";


    auto storeWinPos = [this](){
        settings_->setValue(CFG_WND_POS, position());
    };
    connect(this, &MainWindow::xChanged, storeWinPos);
    connect(this, &MainWindow::yChanged, storeWinPos);

    // Load settings
    setPosition(settings_->value(CFG_WND_POS).toPoint());
    setShowCentered(settings_->value(CFG_CENTERED, DEF_CENTERED).toBool());
    setHideOnFocusLoss(settings_->value(CFG_HIDEONFOCUSLOSS, DEF_HIDEONFOCUSLOSS).toBool());
    setAlwaysOnTop(settings_->value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setHideOnClose(settings_->value(CFG_HIDE_ON_CLOSE, DEF_HIDE_ON_CLOSE).toBool());
    if ( settings_->contains(CFG_STYLEPATH) && QFile::exists(settings_->value(CFG_STYLEPATH).toString()) )
        setSource(settings_->value(CFG_STYLEPATH).toString());
    else {
        setSource(styles_[0].mainComponent);
        settings_->setValue(CFG_STYLEPATH, styles_[0].mainComponent);
    }

    // Reload qml if changed
    connect(&watcher_, &QFileSystemWatcher::fileChanged,
            [this](){
        qDebug() << "QML file reloaded.";
        QUrl url = source();
        setSource(QUrl());
        engine()->clearComponentCache();
        setSource(url);
        watcher_.addPath(url.toString());
    });

    //
    connect(this, &QQuickView::visibilityChanged, [this](QWindow::Visibility visibility){
        if ( visibility == QWindow::Visibility::Hidden )
            if ( showCentered_ ){
                QDesktopWidget *dw = QApplication::desktop();
                setPosition(dw->availableGeometry(dw->screenNumber(QCursor::pos()))
                            .center()-QPoint(width()/2,192));
            }
    });
}


/** ***************************************************************************/
QmlBoxModel::MainWindow::~MainWindow() {
    // Save settings

    qDebug() << "QML Box Model mainwindow destructor called";
}

/** ***************************************************************************/
QString QmlBoxModel::MainWindow::input() {
    QString retVal;
    QMetaObject::invokeMethod(rootObject(), "getInput", Qt::DirectConnection,
                              Q_RETURN_ARG(QString, retVal));
    return retVal;

}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setInput(const QString &input) {
    QMetaObject::invokeMethod(rootObject(), "setInput", Qt::DirectConnection,
                              Q_ARG(QString, input));
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setSource(const QUrl &url) {

    // Apply the source
    QQuickView::setSource(url);

    if ( url.isEmpty() )
        return;

    // Connect the sigals
    QObject *object = rootObject();

    connect(object, SIGNAL(inputChanged(QString)),
            this, SIGNAL(inputChanged(QString)));

    connect(object, SIGNAL(settingsWidgetRequested()),
            this, SIGNAL(settingsWidgetRequested()));

    connect(object, SIGNAL(settingsWidgetRequested()),
            this, SLOT(hide()));


    // Load the theme properties
    settings_->setValue(CFG_STYLEPATH, source().toString());

    QString themeId = QFileInfo(source().toString()).dir().dirName();

    settings_->beginGroup(themeId);
    for (QString &prop : availableProperties()) {
        if (settings_->contains(prop))
            setProperty(prop.toLatin1().data(), settings_->value(prop));
    }

    if ( !watcher_.files().isEmpty() )
        watcher_.removePaths(watcher_.files());
    watcher_.addPath(url.toString());
}


/** ***************************************************************************/
const std::vector<QmlBoxModel::QmlStyleSpec> &QmlBoxModel::MainWindow::availableStyles() const {
    return styles_;
}


/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::availableProperties() {
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availableProperties",
                              Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}


/** ***************************************************************************/
QVariant QmlBoxModel::MainWindow::property(const char *name) const {
    return rootObject()->property(name);
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setProperty(const char *attribute, const QVariant &value) {
    QString themeId = QFileInfo(source().toString()).dir().dirName();
    settings_->setValue(attribute, value);
    rootObject()->setProperty(attribute, value);
    rootObject()->update();
}



/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::availablePresets() {
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availablePresets",
                              Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setPreset(const QString &name){
    QMetaObject::invokeMethod(rootObject(), "setPreset",
                              Q_ARG(QVariant, QVariant::fromValue(name)));

    // Save the theme properties
    QString themeId = QFileInfo(source().toString()).dir().dirName();
    for (QString &prop : availableProperties())
        settings_->setValue(prop, property(prop.toLatin1().data()));
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setModel(QAbstractItemModel *model) {
    model_.setSourceModel(model);
}



/** ***************************************************************************/
bool QmlBoxModel::MainWindow::event(QEvent *event) {
    switch (event->type())
    {
    // Quit on Alt+F4
    case QEvent::Close:
        ( hideOnClose_ ) ? setVisible(false) : qApp->quit();
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
void QmlBoxModel::MainWindow::resizeEvent(QResizeEvent *event) {

#ifdef __unix__
    QObject *rect = rootObject()->findChild<QObject*>("frame");
    if ( rect ){
        // Keep the input shape consistent
        int shape_event_base, shape_error_base;
        if (XShapeQueryExtension(QX11Info::display(), &shape_event_base, &shape_error_base)) {

            Region region = XCreateRegion();
            XRectangle rectangle;
            rectangle.x      = static_cast<int16_t>(rect->property("x").toUInt());
            rectangle.y      = static_cast<int16_t>(rect->property("y").toUInt());
            rectangle.width  = static_cast<uint16_t>(rect->property("width").toUInt());
            rectangle.height = static_cast<uint16_t>(rect->property("height").toUInt());
            XUnionRectWithRegion(&rectangle, region, region);
            XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0, region, ShapeSet);
            XDestroyRegion(region);
        }
    }
#endif

    QQuickView::resizeEvent(event);
}



/** ***************************************************************************/
bool QmlBoxModel::MainWindow::alwaysOnTop() const {
    return flags() & Qt::WindowStaysOnTopHint;
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setAlwaysOnTop(bool alwaysOnTop) {

    settings_->setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop);

    alwaysOnTop
            ? setFlags(flags() | Qt::WindowStaysOnTopHint)
            : setFlags(flags() & ~Qt::WindowStaysOnTopHint);
    // Flags changed. Update
    QQuickView::hide();
}



/** ***************************************************************************/
bool QmlBoxModel::MainWindow::hideOnFocusLoss() const {
    return hideOnFocusLoss_;
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setHideOnFocusLoss(bool hideOnFocusLoss) {
    settings_->setValue(CFG_HIDEONFOCUSLOSS, hideOnFocusLoss);
    hideOnFocusLoss_ = hideOnFocusLoss;
}



/** ***************************************************************************/
bool QmlBoxModel::MainWindow::showCentered() const {
    return showCentered_;
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setShowCentered(bool showCentered) {
    settings_->setValue(CFG_CENTERED, showCentered);
    showCentered_ = showCentered;
}



/** ***************************************************************************/
bool QmlBoxModel::MainWindow::hideOnClose() const {
    return hideOnClose_;
}



/** ***************************************************************************/
void QmlBoxModel::MainWindow::setHideOnClose(bool hideOnClose) {
    settings_->setValue(CFG_HIDE_ON_CLOSE, hideOnClose);
    hideOnClose_ = hideOnClose;
}
