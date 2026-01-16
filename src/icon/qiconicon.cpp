// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "qiconicon.h"
using namespace albert;

QIconIcon::QIconIcon(const QIcon &icon) : icon_(icon) {}

QSize QIconIcon::actualSize(const QSize &device_independent_size, qreal device_pixel_ratio)
{
    //
    // Docs: QIcon::actualSize:
    //
    // > Returns the actual size of the icon for the requested size, mode, and state. The result might be smaller than
    // > requested, but never larger. The returned size is in device-independent pixels (This is relevant for high-dpi
    // > pixmaps.)
    //
    // Tests show it returns device dependent sizes.
    //
    return icon_.actualSize(device_independent_size * device_pixel_ratio) / device_pixel_ratio;
}

QPixmap QIconIcon::pixmap(const QSize &device_independent_size, qreal device_pixel_ratio)
{
    return icon_.pixmap(device_independent_size, device_pixel_ratio);
}

void QIconIcon::paint(QPainter *painter, const QRect &rect) { icon_.paint(painter, rect); }

bool QIconIcon::isNull() { return icon_.isNull(); }
