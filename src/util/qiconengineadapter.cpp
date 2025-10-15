// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "qiconengineadapter.h"
#include "icon.h"
class QPainter;
using namespace std;

QIconEngineAdapter::QIconEngineAdapter(unique_ptr<albert::Icon> icon) :
    icon_(::move(icon))
{}

QIconEngineAdapter::~QIconEngineAdapter() {}

QSize QIconEngineAdapter::actualSize(const QSize &device_dependent_size, QIcon::Mode, QIcon::State)
{
    return device_dependent_size;
}

QPixmap QIconEngineAdapter::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap QIconEngineAdapter::scaledPixmap(const QSize &device_independent_size, QIcon::Mode, QIcon::State, qreal scale)
{
    return icon_->pixmap(device_independent_size, scale);
}

void QIconEngineAdapter::paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State)
{
    icon_->paint(painter, rect);
}

QString QIconEngineAdapter::iconName() { return icon_->toUrl(); }

QIconEngine* QIconEngineAdapter::clone() const { return new QIconEngineAdapter(icon_->clone()); }

bool QIconEngineAdapter::isNull() { return icon_->isNull(); }
