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
#include <QCursor>
#include <QDir>
#include <QDesktopWidget>
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include <QAbstractItemModel>
#include "mainwidget.h"


/** ***************************************************************************/
MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent) {
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

    // Hide list
    ui.proposalList->hide();

    // Settings
    QSettings s;
    _showCentered = s.value(CFG_CENTERED, CFG_CENTERED_DEF).toBool();
    _theme = s.value(CFG_THEME, CFG_THEME_DEF).toString();
    if (!setTheme(_theme)) {
        qFatal("FATAL: Stylefile not found: %s", _theme.toStdString().c_str());
        exit(EXIT_FAILURE);
    }
    if (s.contains(CFG_WND_POS) && s.value(CFG_WND_POS).canConvert(QMetaType::QPoint))
        move(s.value(CFG_WND_POS).toPoint());
}



/** ***************************************************************************/
MainWidget::~MainWidget() {
    // Save settings
    QSettings s;
    s.setValue(CFG_CENTERED, _showCentered);
    s.setValue(CFG_WND_POS, pos());
    s.setValue(CFG_THEME, _theme);
}



/** ***************************************************************************/
void MainWidget::show() {

    ui.inputLine->clear();
    if (_showCentered){
        QDesktopWidget *dw = QApplication::desktop();
        this->move(dw->availableGeometry(dw->screenNumber(QCursor::pos())).center()
                   -QPoint(rect().right()/2,192 ));
    }
    QWidget::show();
    this->raise();
    this->activateWindow();
    ui.inputLine->setFocus();
    emit widgetShown();
}



/** ***************************************************************************/
void MainWidget::hide() {
    QWidget::hide();
    emit widgetHidden();
}



/** ***************************************************************************/
void MainWidget::toggleVisibility() {
    this->isVisible() ? this->hide() : this->show();
}



/** ***************************************************************************/
void MainWidget::setModel(QAbstractItemModel *m) {
    QItemSelectionModel *sm = ui.proposalList->selectionModel();
    ui.proposalList->setModel(m);
    delete sm;
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
    for (QFileInfo fi : themes) {
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



/** ***************************************************************************/
void MainWidget::closeEvent(QCloseEvent *event) {
    event->accept();
    qApp->quit();
}



/** ***************************************************************************/
void MainWidget::keyPressEvent(QKeyEvent *e) {
    // Hide window on escape key
    if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Escape ) {
        hide();
        e->accept();
    }
    QWidget::keyPressEvent(e);
}
