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

#include <QAbstractItemModel>
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QDesktopWidget>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include "mainwindow.h"
#include "roles.hpp"

const QString MainWindow::CFG_WND_POS  = "windowPosition";
const QString MainWindow::CFG_CENTERED = "showCentered";
const bool    MainWindow::DEF_CENTERED = true;
const QString MainWindow::CFG_THEME = "theme";
const QString MainWindow::DEF_THEME = "Bright";
const QString MainWindow::CFG_HIDE_ON_FOCUS_LOSS = "hideOnFocusLoss";
const bool    MainWindow::DEF_HIDE_ON_FOCUS_LOSS = true;
const QString MainWindow::CFG_ALWAYS_ON_TOP = "alwaysOnTop";
const bool    MainWindow::DEF_ALWAYS_ON_TOP = true;
const char*   MainWindow::CFG_MAX_PROPOSALS = "itemCount";
const uint8_t MainWindow::DEF_MAX_PROPOSALS = 5;
const char*   MainWindow::CFG_DISPLAY_SCROLLBAR = "displayScrollbar";
const bool    MainWindow::DEF_DISPLAY_SCROLLBAR = false;
const char*   MainWindow::CFG_DISPLAY_ICONS = "displayIcons";
const bool    MainWindow::DEF_DISPLAY_ICONS = true;
const char*   MainWindow::SETTINGS_SHORTCUT = "Alt+,";


/** ***************************************************************************/
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      actionsShown_(false),
      historyMoveMod_(Qt::ControlModifier) {

	// INITIALIZE UI
    ui.setupUi(this);
    setWindowTitle(qAppName());
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Tool
                   | Qt::WindowStaysOnTopHint
                   | Qt::WindowCloseButtonHint // No close event w/o this
                   | Qt::FramelessWindowHint);

     // Disable tabbing completely
    ui.actionList->setFocusPolicy(Qt::NoFocus);
    ui.proposalList->setFocusPolicy(Qt::NoFocus);

    // Intercept inputline's events (Navigation with keys, pressed modifiers, etc)
    ui.inputLine->installEventFilter(this);
    ui.inputLine->installEventFilter(ui.proposalList);

    // Set stringlistmodel for actions view
    actionsListModel_ = new QStringListModel(this);
    ui.actionList->setModel(actionsListModel_);

    // Hide lists
    ui.actionList->hide();
    ui.proposalList->hide();

    // Settings button
    settingsButton_ = new SettingsButton(this);
    settingsButton_->setObjectName("settingsButton");
    settingsButton_->setFocusPolicy(Qt::NoFocus);
    settingsButton_->setShortcut(QKeySequence(SETTINGS_SHORTCUT));

    // History
    history_ = new History(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                           filePath(QString("history.dat")), this);

    /*
     * Settings
     */

    QSettings s;
    if (s.contains(CFG_WND_POS))
        move(s.value(CFG_WND_POS).toPoint());
    setShowCentered(s.value(CFG_CENTERED, DEF_CENTERED).toBool());
    setHideOnFocusLoss(s.value(CFG_HIDE_ON_FOCUS_LOSS, DEF_HIDE_ON_FOCUS_LOSS).toBool());
    setAlwaysOnTop(s.value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setMaxProposals(s.value(CFG_MAX_PROPOSALS, DEF_MAX_PROPOSALS).toInt());
    setDisplayScrollbar(s.value(CFG_DISPLAY_SCROLLBAR, DEF_DISPLAY_SCROLLBAR).toBool());
    setDisplayIcons(s.value(CFG_DISPLAY_ICONS, DEF_DISPLAY_ICONS).toBool());
    theme_ = s.value(CFG_THEME, DEF_THEME).toString();
    if (!setTheme(theme_)) {
        qFatal("FATAL: Stylefile not found: %s", theme_.toStdString().c_str());
        qApp->quit();
    }
    if (s.contains(CFG_WND_POS) && s.value(CFG_WND_POS).canConvert(QMetaType::QPoint))
        move(s.value(CFG_WND_POS).toPoint());


    /*
     * Signals
     */

    // Hide window, if settings button has been clicked
    connect(settingsButton_, &QPushButton::clicked, this, &MainWindow::hide);

    // Emit settingsWindowRequested, if settings button has been clicked
    connect(settingsButton_, &QPushButton::clicked, this, &MainWindow::settingsWindowRequested);

    // Trigger query, if text changed
    connect(ui.inputLine, &QLineEdit::textChanged, this, &MainWindow::startQuery);

    // Hide the actionview, if text was changed
    connect(ui.inputLine, &QLineEdit::textChanged, this, &MainWindow::hideActions);

    // Reset history, if text changed
    connect(ui.inputLine, &QLineEdit::textEdited, history_, &History::resetIterator);

    // Hide the actionview, if another item gets clicked
    connect(ui.proposalList, &ProposalList::pressed, this, &MainWindow::hideActions);

    // Trigger default action, if item in proposallist was activated
    QObject::connect(ui.proposalList, &ProposalList::activated, [this](const QModelIndex &index){
        ui.proposalList->model()->setData(index, -1, Roles::Activate);
        history_->add(ui.inputLine->text());
    });

    // Trigger alternative action, if item in actionList was activated
    QObject::connect(ui.actionList, &ActionList::activated, [this](const QModelIndex &index){
        ui.proposalList->model()->setData(ui.proposalList->currentIndex(), index.row(), Roles::Activate);
        history_->add(ui.inputLine->text());
    });
}



/** ***************************************************************************/
MainWindow::~MainWindow() {
    // Save settings
    QSettings s;
    s.setValue(CFG_CENTERED, showCentered());
    s.setValue(CFG_HIDE_ON_FOCUS_LOSS, hideOnFocusLoss());
    s.setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop());
    s.setValue(CFG_WND_POS, pos());
    s.setValue(CFG_THEME, theme());
    s.setValue(CFG_MAX_PROPOSALS, maxProposals());
    s.setValue(CFG_DISPLAY_SCROLLBAR, displayScrollbar());
    s.setValue(CFG_DISPLAY_ICONS, displayIcons());
}



/** ***************************************************************************/
void MainWindow::show() {
    ui.inputLine->clear();
    // Move widget after showing it since QWidget::move works only on widgets
    // that have been shown once. Well as long as this does not introduce ugly
    // flicker this may be okay.
    QWidget::show();
    if (showCentered_){
        QDesktopWidget *dw = QApplication::desktop();
        this->move(dw->availableGeometry(dw->screenNumber(QCursor::pos())).center()
                   -QPoint(rect().right()/2,192 ));
    }
    this->raise();
    this->activateWindow();
    ui.inputLine->setFocus();
    emit widgetShown();
}



/** ***************************************************************************/
void MainWindow::hide() {
    QWidget::hide();
    setShowActions(false);
    emit widgetHidden();
}



/** ***************************************************************************/
void MainWindow::toggleVisibility() {
    this->isVisible() ? this->hide() : this->show();
}



/** ***************************************************************************/
void MainWindow::setModel(QAbstractItemModel *m) {
    //TODO Check this
    QItemSelectionModel *sm = ui.proposalList->selectionModel();
    ui.proposalList->setModel(m);
    delete sm;
}



/** ***************************************************************************/
void MainWindow::setShowCentered(bool b) {
    showCentered_ = b;
}



/** ***************************************************************************/
bool MainWindow::showCentered() const {
    return showCentered_;
}



/** ***************************************************************************/
const QString &MainWindow::theme() const {
    return theme_;
}



/** ***************************************************************************/
bool MainWindow::setTheme(const QString &theme) {
    theme_ = theme;
    QFileInfoList themes;
    QStringList themeDirs = QStandardPaths::locateAll(
        QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);
    for (const QDir &d : themeDirs)
        themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
    // Find and apply the theme
    bool success = false;
    for (const QFileInfo &fi : themes) {
        if (fi.baseName() == theme_) {
            QFile f(fi.canonicalFilePath());
            if (f.open(QFile::ReadOnly)) {
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
bool MainWindow::hideOnFocusLoss() const {
    return hideOnFocusLoss_;
}



/** ***************************************************************************/
void MainWindow::setHideOnFocusLoss(bool b) {
    hideOnFocusLoss_ = b;
}



/** ***************************************************************************/
bool MainWindow::alwaysOnTop() const {
    return windowFlags().testFlag(Qt::WindowStaysOnTopHint);
}



/** ***************************************************************************/
void MainWindow::setAlwaysOnTop(bool alwaysOnTop) {
    alwaysOnTop ? setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint)
                : setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
}



/** ***************************************************************************/
void MainWindow::setMaxProposals(uint8_t maxItems) {
    ui.proposalList->setMaxItems(maxItems);
}



/** ***************************************************************************/
bool MainWindow::displayIcons() const {
    return ui.proposalList->displayIcons();
}



/** ***************************************************************************/
void MainWindow::setDisplayIcons(bool value) {
    ui.proposalList->setDisplayIcons(value);
}



/** ***************************************************************************/
bool MainWindow::displayScrollbar() const {
    return ui.proposalList->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff;
}



/** ***************************************************************************/
void MainWindow::setDisplayScrollbar(bool value) {
    ui.proposalList->setVerticalScrollBarPolicy(
                value ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}



/** ***************************************************************************/
uint8_t MainWindow::maxProposals() const {
    return ui.proposalList->maxItems();
}



/** ***************************************************************************/
bool MainWindow::actionsAreShown() const {
    return actionsShown_;
}



/** ***************************************************************************/
void MainWindow::setShowActions(bool showActions) {

    // Show actions
    if ( showActions && !actionsShown_ ) {

        // Skip if nothing selected
        if ( !ui.proposalList->currentIndex().isValid())
            return;

        // Get actions
        actionsListModel_->setStringList(ui.proposalList->model()->data(
                                             ui.proposalList->currentIndex(),
                                             Roles::Actions).toStringList());

        // Skip if actions are empty
        if (actionsListModel_->rowCount() < 1)
            return;

        ui.actionList->setCurrentIndex(actionsListModel_->index(0, 0, ui.actionList->rootIndex()));
        ui.actionList->show();

        // Make actionlist a event proxy
        ui.inputLine->removeEventFilter(ui.proposalList);
        ui.inputLine->installEventFilter(ui.actionList);

        // Finally set the state
        actionsShown_ = true;
    }

    // Hide actions
    if ( !showActions && actionsShown_ ) {

        ui.actionList->hide();

        // Make proposalList a event proxy
        ui.inputLine->removeEventFilter(ui.actionList);
        ui.inputLine->installEventFilter(ui.proposalList);

        // Finally set the state
        actionsShown_ = false;
    }
}


/** ***************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
    qApp->quit();
}



/** ***************************************************************************/
void MainWindow::resizeEvent(QResizeEvent *event) {
    // Let settingsbutton be in top right corner of frame
    settingsButton_->move(ui.frame->geometry().topRight() - QPoint(settingsButton_->width()-1,0));
    QWidget::resizeEvent(event);
}



/** ***************************************************************************/
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    // Move the wisget with the mouse
    this->move(event->globalPos() - clickOffset_);
    QWidget::mouseMoveEvent(event);
}



/** ***************************************************************************/
void MainWindow::mousePressEvent(QMouseEvent *event) {
    // Save the offset on press for movement calculations
    clickOffset_ = event->pos();
    QWidget::mousePressEvent(event);
}



/** ***************************************************************************/
bool MainWindow::eventFilter(QObject *, QEvent *event) {

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Toggle actionsview
        case Qt::Key_Tab:
            setShowActions(!actionsAreShown());
            return true;

        // Hide window on escape key
        case Qt::Key_Escape:
            hide();
            return true;

        case Qt::Key_Up:{
            // Move up in the history
            if ( !ui.proposalList->currentIndex().isValid() // Empty list
                 || keyEvent->modifiers() == historyMoveMod_ // MoveMod (Ctrl) hold
                 || ( !actionsAreShown() // Not in actions state...
                      && ui.proposalList->currentIndex().row()==0 && !keyEvent->isAutoRepeat() ) ){ // ... and first row (non repeat)
                QString next = history_->next();
                if (!next.isEmpty())
                    ui.inputLine->setText(next);
                return false;
            }
        }

        // Move down in the history
        case Qt::Key_Down:{
            if ( !actionsAreShown() && keyEvent->modifiers() == Qt::ControlModifier ) {
                QString prev = history_->prev();
                if (!prev.isEmpty())
                    ui.inputLine->setText(prev);
                return false;
            }
        }
        }
    }
    return false;
}



/** ***************************************************************************/
bool MainWindow::event(QEvent *event) {
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
        if (hideOnFocusLoss_){
            QTimer::singleShot(50, this, &MainWindow::hide);
        }
    }
    return QWidget::event(event);
}
