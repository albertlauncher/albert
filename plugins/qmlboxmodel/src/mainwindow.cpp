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
}

/** ***************************************************************************/
MainWindow::MainWindow(QWindow *parent) : QQuickView(parent) {
    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );


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
        QDirIterator it(pluginDataPath, QDir::Dirs|QDir::NoDotAndDotDot);
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
        QSettings s(qApp->applicationName());
        s.beginGroup(PLUGIN_ID);
        s.setValue(CFG_WND_POS, position());
    };
    connect(this, &MainWindow::xChanged, storeWinPos);
    connect(this, &MainWindow::yChanged, storeWinPos);

    // Load settings
    QSettings s(qApp->applicationName());
    s.beginGroup(PLUGIN_ID);
    setPosition(s.value(CFG_WND_POS).toPoint());
    setShowCentered(s.value(CFG_CENTERED, DEF_CENTERED).toBool());
    setHideOnFocusLoss(s.value(CFG_HIDEONFOCUSLOSS, DEF_HIDEONFOCUSLOSS).toBool());
    setAlwaysOnTop(s.value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    if ( s.contains(CFG_STYLEPATH) && QFile::exists(s.value(CFG_STYLEPATH).toString()) )
        setSource(s.value(CFG_STYLEPATH).toString());
    else {
        setSource(styles_[0].mainComponent);
        s.setValue(CFG_STYLEPATH, styles_[0].mainComponent);
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
MainWindow::~MainWindow() {
    // Save settings

    qDebug() << "QML Box Model mainwindow destructor called";
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
    QSettings s(qApp->applicationName());
    s.beginGroup(PLUGIN_ID);

    s.setValue(CFG_STYLEPATH, source().toString());

    QString themeId = QFileInfo(source().toString()).dir().dirName();
    s.beginGroup(themeId);
    for (QString &prop : availableProperties()) {
        if (s.contains(prop))
            setProperty(prop.toLatin1().data(), s.value(prop));
    }
    s.endGroup();

    if ( !watcher_.files().isEmpty() )
        watcher_.removePaths(watcher_.files());
    watcher_.addPath(url.toString());
}


/** ***************************************************************************/
const std::vector<QmlStyleSpec> &MainWindow::availableStyles() const {
    return styles_;
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
    QString themeId = QFileInfo(source().toString()).dir().dirName();
    QSettings s(qApp->applicationName());
    s.beginGroup(QString("%1/%2").arg(PLUGIN_ID, themeId));
    s.setValue(attribute, value);
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

    // Save the theme properties
    QString themeId = QFileInfo(source().toString()).dir().dirName();
    QSettings s(qApp->applicationName());
    s.beginGroup(QString("%1/%2").arg(PLUGIN_ID, themeId));
    for (QString &prop : availableProperties())
        s.setValue(prop, property(prop.toLatin1().data()));
    s.endGroup();
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

    QSettings s(qApp->applicationName());
    s.beginGroup(PLUGIN_ID);
    s.setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop);

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
    QSettings s(qApp->applicationName());
    s.beginGroup(PLUGIN_ID);
    s.setValue(CFG_HIDEONFOCUSLOSS, hideOnFocusLoss);
    hideOnFocusLoss_ = hideOnFocusLoss;
}



/** ***************************************************************************/
bool MainWindow::showCentered() const {
    return showCentered_;
}



/** ***************************************************************************/
void MainWindow::setShowCentered(bool showCentered) {
    QSettings s(qApp->applicationName());
    s.beginGroup(PLUGIN_ID);
    s.setValue(CFG_CENTERED, showCentered);
    showCentered_ = showCentered;
}
