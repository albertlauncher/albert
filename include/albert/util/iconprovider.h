// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QPixmap>
#include <QSize>
#include <QStringList>

namespace albert{

/// Generic pixmap provider.

class ALBERT_EXPORT IconProvider
{
public:
    IconProvider();
    ~IconProvider();

    /// Pixmap providing function.
    /// See getPixmap(const QString &url, QSize *size, const QSize &requestedSize) const for supported urls.
    /// \param urls The URLs of the pixmap to be created.
    /// \param size Will get assigned the actual size of the pixmap created
    /// \param requestedSize The size the pixmap should have if possible.
    /// \returns The first pixmap available in the urls list. Null pixmap otherwise.
    QPixmap getPixmap(const QStringList &urls, QSize *size, const QSize &requestedSize) const;

    /// Pixmap providing function.
    /// \param size Will get assigned the actual size of the pixmap created
    /// \param requestedSize The size the pixmap should have if possible.
    /// \param url The URL of the pixmap to be created. Supported url schemes:
    /// - **file**: Url to a local image file.
    /// - **qfip**: Uses [QFileIconProvider] to get the icon for the file path.
    /// - **xdg**: performs [freedesktop icon theme specification] lookup (Linux only).
    /// - **qrc** or empty: A QResource file path.
    /// - **gen**: This is a generic icon factory. All parameters are optioal. It draws a
    ///   background circle and renders text on it. Available parameters are:
    ///   - background: The background color (default: none). See also [QColor::fromString].
    ///   - foreground: (default: window text from system palette)
    ///   - text: (default: none) The text to display
    ///   - fontscalar: (default: 1) Scalar for the default font size which is the pixmap height.
    ///
    ///   Examples
    ///
    ///       file:/path/to/local/image/file
    ///       qfip:/some/dir
    ///       qfip:/but/also/fancy/file/types.pdf
    ///       xdg:some-themed-icon-name
    ///       xdg:nautilus
    ///       qrc:nested/path/to/some.svg
    ///       qrc:or-aliased
    ///       :or-this
    ///       gen:?background=blue&foreground=red&text=Hi&fontscalar=0.5
    ///
    /// \returns The pixmap, if available, null pixmap otherwise.
    ///
    /// [freedesktop icon theme specification]: https://specifications.freedesktop.org/icon-theme-spec/latest/
    /// [QFileIconProvider]: https://doc.qt.io/qt/qfileiconprovider.html
    /// [QColor::fromString]: https://doc.qt.io/qt/qcolor.html#fromString
    QPixmap getPixmap(const QString &url, QSize *size, const QSize &requestedSize) const;

    /// Clears the internal icon cache
    void clearCache();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}

