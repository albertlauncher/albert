// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QAbstractItemModel>
#include <QGraphicsDropShadowEffect>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QStandardPaths>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include "core/history.h"
#include "core/itemroles.h"
#include "configwidget.h"
#include "frontendwidget.h"
#include "resultslist.h"
#include "settingsbutton.h"
#include "ui_frontend.h"
#ifdef __unix__
#include <X11/extensions/shape.h>
#undef KeyPress
#undef KeyRelease
#include <QtX11Extras/QX11Info>
#endif


namespace  {

const char*   CFG_WND_POS  = "windowPosition";
const char*   CFG_CENTERED = "showCentered";
const bool    DEF_CENTERED = true;
const char*   CFG_THEME = "theme";
const char*   DEF_THEME = "Bright";
const char*   CFG_HIDE_ON_FOCUS_LOSS = "hideOnFocusLoss";
const bool    DEF_HIDE_ON_FOCUS_LOSS = true;
const char*   CFG_HIDE_ON_CLOSE = "hideOnClose";
const bool    DEF_HIDE_ON_CLOSE = false;
const char*   CFG_CLEAR_ON_HIDE = "clearOnHide";
const bool    DEF_CLEAR_ON_HIDE = false;
const char*   CFG_ALWAYS_ON_TOP = "alwaysOnTop";
const bool    DEF_ALWAYS_ON_TOP = true;
const char*   CFG_MAX_RESULTS = "itemCount";
const uint8_t DEF_MAX_RESULTS = 5;
const char*   CFG_DISPLAY_SCROLLBAR = "displayScrollbar";
const bool    DEF_DISPLAY_SCROLLBAR = false;
const char*   CFG_DISPLAY_ICONS = "displayIcons";
const bool    DEF_DISPLAY_ICONS = true;
const char*   CFG_DISPLAY_SHADOW = "displayShadow";
const bool    DEF_DISPLAY_SHADOW = true;

}

/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/

class WidgetBoxModel::FrontendWidget::Private
{
public:

    /** The name of the current theme */
    QString theme_;

    /** The offset from cursor to topleft. Used when the window is dagged */
    QPoint clickOffset_;

    /** The model of the action list view */
    QStringListModel *actionsListModel_;

    /** The button to open the settings dialog */
    SettingsButton *settingsButton_;

    /** The input history */
    History *history_;

    /** The modifier used to navigate directly in the history */
    Qt::KeyboardModifier historyMoveMod_;

    /** The form of the main app */
    Ui::MainWindow ui;

    /** Indicates that the app should be shown centered */
    bool showCentered_;

    /** Indicates that the app should be hidden on focus loss */
    bool hideOnFocusLoss_;

    /** Indicates that the app should be hidden on close event */
    bool hideOnClose_;

    /** Indcates the state that the app is in */
    bool actionsShown_;

    /** Indcates that a shadow should be drawn */
    bool displayShadow_;

    /** Indcates that the inputline should be cleared on hide */
    bool clearOnHide_;

};

/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
WidgetBoxModel::FrontendWidget::FrontendWidget() : d(new Private) {

    d->actionsShown_ = false;
    d->historyMoveMod_ = Qt::ControlModifier;

	// INITIALIZE UI
    d->ui.setupUi(this);
//    setWindowIcon(qApp->windowIcon());
    setWindowTitle(qAppName());
    setWindowFlags(Qt::Tool
                   | Qt::WindowCloseButtonHint // No close event w/o this
                   | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(20);
    effect->setColor(QColor(0, 0, 0 , 192 ))  ;
    effect->setXOffset(0.0);
    effect->setYOffset(3.0);
    setGraphicsEffect(effect);

     // Disable tabbing completely
    d->ui.actionList->setFocusPolicy(Qt::NoFocus);
    d->ui.resultsList->setFocusPolicy(Qt::NoFocus);

    // Set initial event filter pipeline: window -> resultslist -> lineedit
    d->ui.inputLine->installEventFilter(d->ui.resultsList);
    d->ui.inputLine->installEventFilter(this);

    // Set stringlistmodel for actions view
    d->actionsListModel_ = new QStringListModel(this);
    d->ui.actionList->setModel(d->actionsListModel_);

    // Hide lists
    d->ui.actionList->hide();
    d->ui.resultsList->hide();

    // Settings button
    d->settingsButton_ = new SettingsButton(this);
    d->settingsButton_->setObjectName("settingsButton");
    d->settingsButton_->setFocusPolicy(Qt::NoFocus);
    d->settingsButton_->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Context menu of settingsbutton
    QAction *action = new QAction("Settings", d->settingsButton_);
    action->setShortcuts({QKeySequence("Ctrl+,"), QKeySequence("Alt+,")});
    connect(action, &QAction::triggered, this, &FrontendWidget::hide);
    connect(action, &QAction::triggered, this, &FrontendWidget::settingsWidgetRequested);
    connect(d->settingsButton_, &QPushButton::clicked, action, &QAction::trigger);
    d->settingsButton_->addAction(action);

    action = new QAction("Hide", d->settingsButton_);
    action->setShortcut(QKeySequence("Esc"));
    connect(action, &QAction::triggered, this, &FrontendWidget::hide);
    d->settingsButton_->addAction(action);

    action = new QAction("Separator", d->settingsButton_);
    action->setSeparator(true);
    d->settingsButton_->addAction(action);

    action = new QAction("Quit", d->settingsButton_);
    action->setShortcut(QKeySequence("Alt+F4"));
    connect(action, &QAction::triggered, qApp, &QApplication::quit);
    d->settingsButton_->addAction(action);

    // History
    d->history_ = new History(this);

    /*
     * Settings
     */

    QSettings s(qApp->applicationName());
    setShowCentered(s.value(CFG_CENTERED, DEF_CENTERED).toBool());
    if (!showCentered() && s.contains(CFG_WND_POS) && s.value(CFG_WND_POS).canConvert(QMetaType::QPoint))
        move(s.value(CFG_WND_POS).toPoint());
    setHideOnFocusLoss(s.value(CFG_HIDE_ON_FOCUS_LOSS, DEF_HIDE_ON_FOCUS_LOSS).toBool());
    setHideOnClose(s.value(CFG_HIDE_ON_CLOSE, DEF_HIDE_ON_CLOSE).toBool());
    setClearOnHide(s.value(CFG_CLEAR_ON_HIDE, DEF_CLEAR_ON_HIDE).toBool());
    setAlwaysOnTop(s.value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setMaxResults(static_cast<u_int8_t>(s.value(CFG_MAX_RESULTS, DEF_MAX_RESULTS).toInt()));
    setDisplayScrollbar(s.value(CFG_DISPLAY_SCROLLBAR, DEF_DISPLAY_SCROLLBAR).toBool());
    setDisplayIcons(s.value(CFG_DISPLAY_ICONS, DEF_DISPLAY_ICONS).toBool());
    setDisplayShadow(s.value(CFG_DISPLAY_SHADOW, DEF_DISPLAY_SHADOW).toBool());
    d->theme_ = s.value(CFG_THEME, DEF_THEME).toString();
    if (!setTheme(d->theme_))
        qFatal("FATAL: Stylefile not found: %s", d->theme_.toStdString().c_str());


    /*
     * Signals
     */

    // Trigger query, if text changed
    connect(d->ui.inputLine, &QLineEdit::textChanged, this, &FrontendWidget::inputChanged);

    // Hide the actionview, if text was changed
    connect(d->ui.inputLine, &QLineEdit::textChanged, this, &FrontendWidget::hideActions);

    // Reset history, if text was manually changed
    connect(d->ui.inputLine, &QLineEdit::textEdited, d->history_, &History::resetIterator);

    // Hide the actionview, if another item gets clicked
    connect(d->ui.resultsList, &ResultsList::pressed, this, &FrontendWidget::hideActions);

    // Trigger default action, if item in resultslist was activated
    QObject::connect(d->ui.resultsList, &ResultsList::activated, [this](const QModelIndex &index){

        switch (qApp->queryKeyboardModifiers()) {
        case Qt::MetaModifier: // Default fallback action (Meta)
            d->ui.resultsList->model()->setData(index, -1, ItemRoles::FallbackRole);
            break;
        default: // DefaultAction
            d->ui.resultsList->model()->setData(index, -1, ItemRoles::ActionRole);
            break;
        }

        // Do not move this up! (Invalidates index)
        d->history_->add(d->ui.inputLine->text());
        this->setVisible(false);
        d->ui.inputLine->clear();
    });

    // Trigger alternative action, if item in actionList was activated
    QObject::connect(d->ui.actionList, &ActionList::activated, [this](const QModelIndex &index){
        d->history_->add(d->ui.inputLine->text());
        d->ui.resultsList->model()->setData(d->ui.resultsList->currentIndex(), index.row(), ItemRoles::AltActionRole);
        this->setVisible(false);
        d->ui.inputLine->clear();
    });
}


/** ***************************************************************************/
WidgetBoxModel::FrontendWidget::~FrontendWidget() {
    // Needed since default dtor of unique ptr in the header has to know the type
    qDebug() << "Widget Box Model mainwindow destructor called";
}


/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::isVisible() {
    return QWidget::isVisible();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setVisible(bool visible) {

    // Skip if nothing to do
    if ( (isVisible() && visible) || !(isVisible() || visible) )
        return;

    QWidget::setVisible(visible);

    if (visible) {
        // Move widget after showing it since QWidget::move works only on widgets
        // that have been shown once. Well as long as this does not introduce ugly
        // flicker this may be okay.
        if (d->showCentered_){
            QDesktopWidget *dw = QApplication::desktop();
            this->move(dw->screenGeometry(dw->screenNumber(QCursor::pos())).center()
                       -QPoint(rect().right()/2,192 ));
        }
        this->raise();
        this->activateWindow();
        d->ui.inputLine->setFocus();
        emit widgetShown();
    } else {
        setShowActions(false);
        d->history_->resetIterator();
        ( d->clearOnHide_ ) ? d->ui.inputLine->clear() : d->ui.inputLine->selectAll();
        emit widgetHidden();
    }
}


/** ***************************************************************************/
QString WidgetBoxModel::FrontendWidget::input() {
    return d->ui.inputLine->text();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setInput(const QString &input) {
    d->ui.inputLine->setText(input);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setModel(QAbstractItemModel *m) {
    d->ui.resultsList->setModel(m);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setShowCentered(bool b) {
    QSettings(qApp->applicationName()).setValue(CFG_CENTERED, b);
    d->showCentered_ = b;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::showCentered() const {
    return d->showCentered_;
}



/** ***************************************************************************/
const QString &WidgetBoxModel::FrontendWidget::theme() const {
    return d->theme_;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::setTheme(const QString &theme) {
    d->theme_ = theme;
    QFileInfoList themes;


    QStringList pluginDataPaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                            "org.albert.frontend.boxmodel.widgets",
                                                            QStandardPaths::LocateDirectory);

    for (const QString &pluginDataPath : pluginDataPaths)
        themes << QDir(QString("%1/themes").arg(pluginDataPath))
                  .entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);

    // Find and apply the theme
    bool success = false;
    for (const QFileInfo &fi : themes) {
        if (fi.baseName() == d->theme_) {
            QFile f(fi.canonicalFilePath());
            if (f.open(QFile::ReadOnly)) {
                QSettings(qApp->applicationName()).setValue(CFG_THEME, d->theme_);
                setStyleSheet(f.readAll());
                f.close();
                success = true;
                break;
            }
        }
    }
    return success;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::hideOnFocusLoss() const {
    return d->hideOnFocusLoss_;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setHideOnFocusLoss(bool b) {
    QSettings(qApp->applicationName()).setValue(CFG_HIDE_ON_FOCUS_LOSS, b);
    d->hideOnFocusLoss_ = b;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::hideOnClose() const {
    return d->hideOnClose_;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setHideOnClose(bool b) {
    QSettings(qApp->applicationName()).setValue(CFG_HIDE_ON_CLOSE, b);
    d->hideOnClose_ = b;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::clearOnHide() const {
    return d->clearOnHide_;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setClearOnHide(bool b) {
    QSettings(qApp->applicationName()).setValue(CFG_CLEAR_ON_HIDE, b);
    d->clearOnHide_ = b;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::alwaysOnTop() const {
    return windowFlags().testFlag(Qt::WindowStaysOnTopHint);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setAlwaysOnTop(bool alwaysOnTop) {
    QSettings(qApp->applicationName()).setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop);
    // TODO: QT_MINREL 5.7 setFlag
    alwaysOnTop ? setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint)
                : setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setMaxResults(uint8_t maxItems) {
    QSettings(qApp->applicationName()).setValue(CFG_MAX_RESULTS, maxItems);
    d->ui.resultsList->setMaxItems(maxItems);
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::displayIcons() const {
    return d->ui.resultsList->displayIcons();
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setDisplayIcons(bool value) {
    QSettings(qApp->applicationName()).setValue(CFG_DISPLAY_ICONS, value);
    d->ui.resultsList->setDisplayIcons(value);
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::displayScrollbar() const {
    return d->ui.resultsList->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setDisplayScrollbar(bool value) {
    QSettings(qApp->applicationName()).setValue(CFG_DISPLAY_SCROLLBAR, value);
    d->ui.resultsList->setVerticalScrollBarPolicy(
                value ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::displayShadow() const {
    return d->displayShadow_;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setDisplayShadow(bool value) {
    QSettings(qApp->applicationName()).setValue(CFG_DISPLAY_SHADOW, value);
    d->displayShadow_ = value;
    graphicsEffect()->setEnabled(value);
    value ? setContentsMargins(20,20,20,20) : setContentsMargins(0,0,0,0);
}



/** ***************************************************************************/
uint8_t WidgetBoxModel::FrontendWidget::maxResults() const {
    return d->ui.resultsList->maxItems();
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::actionsAreShown() const {
    return d->actionsShown_;
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::setShowActions(bool showActions) {

    // Show actions
    if ( showActions && !d->actionsShown_ ) {

        // Skip if nothing selected
        if ( !d->ui.resultsList->currentIndex().isValid())
            return;

        // Get actions
        d->actionsListModel_->setStringList(d->ui.resultsList->model()->data(
                                                d->ui.resultsList->currentIndex(),
                                                ItemRoles::AltActionRole).toStringList());

        // Skip if actions are empty
        if (d->actionsListModel_->rowCount() < 1)
            return;

        d->ui.actionList->setCurrentIndex(d->actionsListModel_->index(0, 0, d->ui.actionList->rootIndex()));
        d->ui.actionList->show();

        // Change event filter pipeline: window -> _action_list -> lineedit
        d->ui.inputLine->removeEventFilter(this);
        d->ui.inputLine->removeEventFilter(d->ui.resultsList);
        d->ui.inputLine->installEventFilter(d->ui.actionList);
        d->ui.inputLine->installEventFilter(this);

        // Finally set the state
        d->actionsShown_ = true;
    }

    // Hide actions
    if ( !showActions && d->actionsShown_ ) {

        d->ui.actionList->hide();

        // Change event filter pipeline: window -> resultslist -> lineedit
        d->ui.inputLine->removeEventFilter(this);
        d->ui.inputLine->removeEventFilter(d->ui.actionList);
        d->ui.inputLine->installEventFilter(d->ui.resultsList);
        d->ui.inputLine->installEventFilter(this);

        // Finally set the state
        d->actionsShown_ = false;
    }
}



/** ***************************************************************************/
QWidget *WidgetBoxModel::FrontendWidget::widget(QWidget *parent) {
    return new ConfigWidget(this, parent);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::closeEvent(QCloseEvent *event) {
    event->accept();
    if (!d->hideOnClose_)
        qApp->quit();
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::resizeEvent(QResizeEvent *event) {

    // Let settingsbutton be in top right corner of frame
    d->settingsButton_->move(d->ui.frame->geometry().topRight() - QPoint(d->settingsButton_->width()-1,0));

#ifdef __unix__
    // Keep the input shape consistent
    int shape_event_base, shape_error_base;
    if (XShapeQueryExtension(QX11Info::display(), &shape_event_base, &shape_error_base)) {

        Region region = XCreateRegion();
        XRectangle rectangle;
        rectangle.x      = static_cast<int16_t>(d->ui.frame->geometry().x());
        rectangle.y      = static_cast<int16_t>(d->ui.frame->geometry().y());
        rectangle.width  = static_cast<uint16_t>(d->ui.frame->geometry().width());
        rectangle.height = static_cast<uint16_t>(d->ui.frame->geometry().height());
        XUnionRectWithRegion(&rectangle, region, region);
        XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0, region, ShapeSet);
        XDestroyRegion(region);
    }
#endif

    QWidget::resizeEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::mouseMoveEvent(QMouseEvent *event) {
    // Move the widget with the mouse
    move(event->globalPos() - d->clickOffset_);
    QWidget::mouseMoveEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::mousePressEvent(QMouseEvent *event) {
    // Save the offset on press for movement calculations
    d->clickOffset_ = event->pos();
    QWidget::mousePressEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::FrontendWidget::mouseReleaseEvent(QMouseEvent *event) {
    // Save the window position ()
    QSettings(qApp->applicationName()).setValue(CFG_WND_POS, pos());
    QWidget::mousePressEvent(event);
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::eventFilter(QObject *, QEvent *event) {

    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Toggle insert completion string
        case Qt::Key_Tab:
            if ( d->ui.resultsList->currentIndex().isValid() )
                d->ui.inputLine->setText(
                            d->ui.resultsList->model()->data(
                                d->ui.resultsList->currentIndex(), ItemRoles::CompletionRole
                                ).toString()
                            );
            return true;

        case Qt::Key_Alt:
            setShowActions(true);
            return true;

        case Qt::Key_Up:{
            // Move up in the history
            if ( !d->ui.resultsList->currentIndex().isValid() // Empty list
                 || keyEvent->modifiers() == d->historyMoveMod_ // MoveMod (Ctrl) hold
                 || ( !actionsAreShown() // Not in actions state...
                      && d->ui.resultsList->currentIndex().row()==0 && !keyEvent->isAutoRepeat() ) ){ // ... and first row (non repeat)
                QString next = d->history_->next();
                if (!next.isEmpty())
                    d->ui.inputLine->setText(next);
                return true;
            }
        }

        // Move down in the history
        case Qt::Key_Down:{
            if ( !actionsAreShown() && keyEvent->modifiers() == Qt::ControlModifier ) {
                QString prev = d->history_->prev();
                if (!prev.isEmpty())
                    d->ui.inputLine->setText(prev);
                return true;
            }
        }
        }
    }

    if ( event->type() == QEvent::KeyRelease ) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Alt:
            setShowActions(false);
            return true;
        }
    }

    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        if ( wheelEvent->angleDelta().y() > 0 ) {
            QString next = d->history_->next();
            if (!next.isEmpty())
                d->ui.inputLine->setText(next);
        } else {
            QString prev = d->history_->prev();
            if (!prev.isEmpty())
                d->ui.inputLine->setText(prev);
        }
    }

    return false;
}



/** ***************************************************************************/
bool WidgetBoxModel::FrontendWidget::event(QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate) {
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
        if (d->hideOnFocusLoss_){
            // Note fix if least LTS goes beyond Qt5.4
            // QTimer::singleShot(50, this, &WidgetBoxModel::Frontend::hide);
            QTimer::singleShot(50, this, SLOT(hide()));
        }
    }
    return QWidget::event(event);
}
