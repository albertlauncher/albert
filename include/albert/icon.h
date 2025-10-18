// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <memory>
class QPainter;
class QRect;
class QSize;
class QPixmap;
class QString;

namespace albert
{

///
/// Abstract icon engine.
///
/// \ingroup core
///
class ALBERT_EXPORT Icon
{
public:

    ///
    /// Destructs the icon.
    ///
    virtual ~Icon() = default;

    ///
    /// Returns a clone of this icon.
    ///
    virtual std::unique_ptr<Icon> clone() const = 0;

    ///
    /// Returns the device independent size of the available icon for the given
    /// _device_independent_size_ and _device_pixel_ratio_.
    ///
    /// The base implementations returns _device_independent_size_.
    ///
    virtual QSize actualSize(const QSize &device_independent_size, double device_pixel_ratio);

    ///
    /// Returns a pixmap for the requested _device_independent_size_ and _device_pixel_ratio_.
    ///
    /// The base implementation creates a transparent pixmap of \ref actualSize and calls \ref paint on it.
    ///
    virtual QPixmap pixmap(const QSize &device_independent_size, double device_pixel_ratio);

    ///
    /// Uses the given _painter_ to paint the icon into the rectangle _rect_.
    ///
    virtual void paint(QPainter *painter, const QRect &rect) = 0;

    ///
    /// Returns `true` if the icon is valid; otherwise returns `false`.
    ///
    /// The base implementation returns `false`.
    ///
    virtual bool isNull();

    ///
    /// Returns a URL representation of the icon.
    ///
    virtual QString toUrl() const = 0;

    ///
    /// Returns the cache key of the icon.
    ///
    /// The base implementation calls \ref toUrl. Reimplement to get faster lookups.
    ///
    virtual QString cacheKey();

};

}  // namespace albert
