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

#include "settingsbutton.h"
#include <QRect>
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QStyleOptionButton>
#include <QtSvg/QSvgRenderer>

/** ***************************************************************************/
WidgetBoxModel::SettingsButton::SettingsButton(QWidget *parent) : QPushButton(parent) {
    animation_ = new QPropertyAnimation(this, "angle");
    animation_->setDuration(10000);
    animation_->setStartValue(0);
    animation_->setEndValue(360);
    animation_->setLoopCount(-1);
    animation_->start();
    connect(animation_, &QPropertyAnimation::valueChanged, this, static_cast<void (QWidget::*)()>(&QWidget::update));

    svgRenderer_ = new QSvgRenderer(QString(":gear"));

    setCursor(Qt::PointingHandCursor);
}



/** ***************************************************************************/
WidgetBoxModel::SettingsButton::~SettingsButton() {
    delete animation_;
    delete svgRenderer_;
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::hideEvent(QHideEvent *event) {
    animation_->stop();
    QPushButton::hideEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::showEvent(QShowEvent *event) {
    animation_->start();
    QPushButton::showEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    QStyleOptionButton option;
    option.initFrom(this);
    QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

    // Prepare image in pixmap using mask
    QPixmap gearPixmap(contentRect.size());
    QRectF pixmapRect(QPoint(), contentRect.size());
    gearPixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&gearPixmap);
    pixmapPainter.translate(pixmapRect.center());
    pixmapPainter.rotate(angle_);
    svgRenderer_->render(&pixmapPainter, pixmapRect.translated(-pixmapRect.center()));
    pixmapPainter.resetTransform();
    pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pixmapPainter.fillRect(pixmapRect, option.palette.windowText().color());

    // Draw pixmap on button
    QPainter painter(this);
    painter.drawPixmap(contentRect, gearPixmap);

}
