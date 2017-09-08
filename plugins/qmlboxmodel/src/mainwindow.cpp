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
#include "frontendplugin.h"
#ifdef __unix__
#include "xcb/xcb.h"
#include <X11/extensions/shape.h>
#undef KeyPress
#undef KeyRelease
#undef FocusOut
#undef Status
#include <QtX11Extras/QX11Info>
#endif

namespace {
const QString CFG_CENTERED        = "showCentered";
const bool    DEF_CENTERED        = true;
const QString CFG_HIDEONFOCUSLOSS = "hideOnFocusLoss";
const bool    DEF_HIDEONFOCUSLOSS = true;
const QString CFG_ALWAYS_ON_TOP   = "alwaysOnTop";
const bool    DEF_ALWAYS_ON_TOP   = true;
const char*   CFG_HIDE_ON_CLOSE   = "hideOnClose";
const bool    DEF_HIDE_ON_CLOSE   = false;
const QString CFG_STYLEPATH       = "stylePath";
const QString CFG_WND_POS         = "windowPosition";
const QString PLUGIN_ID           = "org.albert.frontend.boxmodel.qml";
const QString STYLE_MAIN_NAME     = "MainComponent.qml";
const QString STYLE_CONFIG_NAME   = "style_properties.ini";
const QString PREF_OBJ_NAME       = "preferences";
const QString FRAME_OBJ_NAME      = "frame";
}

/** ***************************************************************************/
QmlBoxModel::MainWindow::MainWindow(FrontendPlugin *plugin, QWindow *parent) : QQuickView(parent) {
    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    plugin_ = plugin;

    // Set qml environment
    rootContext()->setContextProperty("mainWindow", this);
    rootContext()->setContextProperty("history", &history_);
    rootContext()->setContextProperty("resultsModel", &model_);

    // Quit application when qml signals quit
    connect(engine(), SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));

    // When component is ready load the saved properties
    connect(this, &QQuickView::statusChanged, this, [this](QQuickView::Status status){
        if ( status == QQuickView::Status::Ready ){

            // Get root object
            if (!rootObject()){
                qWarning() << "Could not retrieve settableProperties: There is no root object.";
                return;
            }

            // Forward signals
            connect(rootObject(), SIGNAL(inputChanged(QString)),
                    this, SIGNAL(inputChanged(QString)));

            connect(rootObject(), SIGNAL(settingsWidgetRequested()),
                    this, SIGNAL(settingsWidgetRequested()));

            connect(rootObject(), SIGNAL(settingsWidgetRequested()),
                    this, SLOT(hide()));

            // Get preferences object
            QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
            if (!preferencesObject){
                qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                                 "There is no object named '%1'.").arg(PREF_OBJ_NAME));
                return;
            }

            // Load the style properties in the group of this style id
            QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
            s.beginGroup(QFileInfo(source().toString()).dir().dirName());
            for (const QString &prop : settableProperties())
                if (s.contains(prop))
                    preferencesObject->setProperty(prop.toLatin1().data(), s.value(prop));
        }
    });

    // Reload if source file changed
    connect(&watcher_, &QFileSystemWatcher::fileChanged,
            [this](){
        qDebug() << "QML file reloaded.";
        QUrl url = source();
        setSource(QUrl());
        engine()->clearComponentCache();
        setSource(url);
        watcher_.addPath(url.toString());
    });

    // Center window between each hide and show
    connect(this, &QQuickView::visibilityChanged, [this](QWindow::Visibility visibility){
        if ( visibility == QWindow::Visibility::Hidden )
            if ( showCentered_ ){
                QDesktopWidget *dw = QApplication::desktop();
                setPosition(dw->availableGeometry(dw->screenNumber(QCursor::pos()))
                            .center()-QPoint(width()/2,256));
            }
    });

    QStringList pluginDataPaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                            "org.albert.frontend.boxmodel.qml",
                                                            QStandardPaths::LocateDirectory);

//    // Add the shared modules to the lookup path
//    for (const QString &pluginDataPath : pluginDataPaths){
//        QDir pluginDataDir = QDir(pluginDataPath);
//        if ( pluginDataDir.exists("shared") )
//            engine()->addImportPath(pluginDataDir.filePath("shared"));
//    }

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
        plugin_->settings().setValue(CFG_WND_POS, position());
    };
    connect(this, &MainWindow::xChanged, storeWinPos);
    connect(this, &MainWindow::yChanged, storeWinPos);

    // Load window settings
    setPosition(plugin_->settings().value(CFG_WND_POS).toPoint());
    setShowCentered(plugin_->settings().value(CFG_CENTERED, DEF_CENTERED).toBool());
    setHideOnFocusLoss(plugin_->settings().value(CFG_HIDEONFOCUSLOSS, DEF_HIDEONFOCUSLOSS).toBool());
    setAlwaysOnTop(plugin_->settings().value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setHideOnClose(plugin_->settings().value(CFG_HIDE_ON_CLOSE, DEF_HIDE_ON_CLOSE).toBool());
    if ( plugin_->settings().contains(CFG_STYLEPATH) && QFile::exists(plugin_->settings().value(CFG_STYLEPATH).toString()) )
        setSource(plugin_->settings().value(CFG_STYLEPATH).toString());
    else {
        setSource(styles_[0].mainComponent);
        plugin_->settings().setValue(CFG_STYLEPATH, styles_[0].mainComponent);
    }

}


/** ***************************************************************************/
QmlBoxModel::MainWindow::~MainWindow() {

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

    // Save the theme
    plugin_->settings().setValue(CFG_STYLEPATH, source().toString());

    // Watch this source file for modifications
    if ( !watcher_.files().isEmpty() )
        watcher_.removePaths(watcher_.files());
    watcher_.addPath(url.toString());
}


/** ***************************************************************************/
const std::vector<QmlBoxModel::QmlStyleSpec> &QmlBoxModel::MainWindow::availableStyles() const {
    return styles_;
}


/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::settableProperties() {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QStringList();
    }

    // Get preferences object
    const QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject){
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return QStringList();
    }

    // Get preferences object's meta object (Reflection yieh…)
    const QMetaObject *preferencesMetaObject = preferencesObject->metaObject();
    if (!preferencesMetaObject){
        qWarning() << "Could not retrieve settableProperties: Fetching MetaObject failed.";
        return QStringList();
    }

    // Get all properties of the object
    QStringList settableProperties;
    for (int i = 0; i < preferencesMetaObject->propertyCount(); i++)
        settableProperties.append(preferencesMetaObject->property(i).name());

    // QtObject type has a single property "objectName". Remove it.
    settableProperties.removeAll("objectName");

    return settableProperties;
}


/** ***************************************************************************/
QVariant QmlBoxModel::MainWindow::property(const char *name) const {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QVariant();
    }

    // Get preferences object
    const QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject){
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return QVariant();
    }

    return preferencesObject->property(name);
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setProperty(const char *attribute, const QVariant &value) {

    // Create the settings instance of the decicated file in config location
    QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
    s.beginGroup(QFileInfo(source().toString()).dir().dirName());
    s.setValue(attribute, value);

    // Get root object
    if (!rootObject()) {
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return;
    }

    // Get preferences object
    QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject) {
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return;
    }

    // Set the property
    preferencesObject->setProperty(attribute, value);
}


/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::availableThemes() {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QStringList();
    }

    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availableThemes", Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setTheme(const QString &name){

    // Get root object
    if (!rootObject()) {
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return;
    }

    // Let qml apply the theme
    QMetaObject::invokeMethod(rootObject(), "setTheme", Q_ARG(QVariant, QVariant::fromValue(name)));

    // Save all current poperties in the group with this style id
    QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
    QString styleId = QFileInfo(source().toString()).dir().dirName();
    s.beginGroup(styleId);
    for (const QString &prop : settableProperties())
        s.setValue(prop, property(prop.toLatin1().data()));
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
    default:break;
    }
    return QQuickView::event(event);
}


#ifdef Q_OS_LINUX
/** ****************************************************************************
 * @brief MainWidget::nativeEvent
 *
 * The purpose of this function is to hide in special casesonly.
 */
bool QmlBoxModel::MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *)
{
    if (eventType == "xcb_generic_event_t")
    {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
        switch (event->response_type & 127)
        {
        case XCB_FOCUS_OUT: {
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

             Another much simpler but less elegant solution is to delay the
             hiding a few milliseconds, so that the hotkey event will always be
             handled first. */

            xcb_focus_out_event_t *fe = reinterpret_cast<xcb_focus_out_event_t*>(event);
//            qDebug() << "MainWidget::nativeEvent::XCB_FOCUS_OUT\t";
//            switch (fe->mode) {
//            case XCB_NOTIFY_MODE_NORMAL: qDebug() << "XCB_NOTIFY_MODE_NORMAL";break;
//            case XCB_NOTIFY_MODE_GRAB: qDebug() << "XCB_NOTIFY_MODE_GRAB";break;
//            case XCB_NOTIFY_MODE_UNGRAB: qDebug() << "XCB_NOTIFY_MODE_UNGRAB";break;
//            case XCB_NOTIFY_MODE_WHILE_GRABBED: qDebug() << "XCB_NOTIFY_MODE_WHILE_GRABBED";break;
//            }
//            switch (fe->detail) {
//            case XCB_NOTIFY_DETAIL_ANCESTOR: qDebug() << "ANCESTOR";break;
//            case XCB_NOTIFY_DETAIL_INFERIOR: qDebug() << "INFERIOR";break;
//            case XCB_NOTIFY_DETAIL_NONE: qDebug() << "NONE";break;
//            case XCB_NOTIFY_DETAIL_NONLINEAR: qDebug() << "NONLINEAR";break;
//            case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL: qDebug() << "NONLINEAR_VIRTUAL";break;
//            case XCB_NOTIFY_DETAIL_POINTER: qDebug() << "POINTER";break;break;
//            case XCB_NOTIFY_DETAIL_POINTER_ROOT: qDebug() << "POINTER_ROOT";
//            case XCB_NOTIFY_DETAIL_VIRTUAL: qDebug() << "VIRTUAL";break;
//            }
            if ((/*(fe->mode==XCB_NOTIFY_MODE_GRAB && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR) ||*/
                 (fe->mode==XCB_NOTIFY_MODE_NORMAL && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR )) &&
                    hideOnFocusLoss_)
                hide();
            return true;
         }
        }
    }
    return false;
}
#endif


/** ***************************************************************************/
void QmlBoxModel::MainWindow::resizeEvent(QResizeEvent *event) {

    QQuickView::resizeEvent(event);

#ifdef __unix__

    // Get root object
    if (!rootObject()) {
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return;
    }

    // Get preferences object
    QObject *frameObject = rootObject()->findChild<QObject*>(FRAME_OBJ_NAME, Qt::FindChildrenRecursively);
    if (frameObject) {
        // Keep the input shape consistent
        int shape_event_base, shape_error_base;
        if (XShapeQueryExtension(QX11Info::display(), &shape_event_base, &shape_error_base)) {

            Region region = XCreateRegion();
            XRectangle rectangle;
            rectangle.x      = static_cast<int16_t>(frameObject->property("x").toUInt());
            rectangle.y      = static_cast<int16_t>(frameObject->property("y").toUInt());
            rectangle.width  = static_cast<uint16_t>(frameObject->property("width").toUInt());
            rectangle.height = static_cast<uint16_t>(frameObject->property("height").toUInt());
            XUnionRectWithRegion(&rectangle, region, region);
            XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0, region, ShapeSet);
            XDestroyRegion(region);
        }
    } else
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(FRAME_OBJ_NAME));
#endif

}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::alwaysOnTop() const {
    return flags() & Qt::WindowStaysOnTopHint;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setAlwaysOnTop(bool alwaysOnTop) {

    plugin_->settings().setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop);

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
    plugin_->settings().setValue(CFG_HIDEONFOCUSLOSS, hideOnFocusLoss);
    hideOnFocusLoss_ = hideOnFocusLoss;
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::showCentered() const {
    return showCentered_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setShowCentered(bool showCentered) {
    plugin_->settings().setValue(CFG_CENTERED, showCentered);
    showCentered_ = showCentered;
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::hideOnClose() const {
    return hideOnClose_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setHideOnClose(bool hideOnClose) {
    plugin_->settings().setValue(CFG_HIDE_ON_CLOSE, hideOnClose);
    hideOnClose_ = hideOnClose;
}
