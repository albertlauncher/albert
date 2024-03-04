// Copyright (c) 2022-2024 Manuel Schneider

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
    /// - `<path>` or `file:<path>` Use the file at path as icon.
    /// - `:<path>` or `qrc:<path>` Use the file at path in the [resource collection] as icon.
    /// - `qfip:<path>` Use [QFileIconProvider] to get a generic icon for the file at <path>.
    /// - `qsp:<pixmap enumerator>` Get an icon from [QStyle::StandardPixmap enum].
    /// - `xdg:<icon name>` Performs [freedesktop icon theme specification] lookup (on supported
    ///   platforms only) to get an icon.
    /// - `gen:<>` Generate an icon on the fly. Supports drawing a background circle and renders
    ///   text on it. All parameters are optional. Available parameters are:
    ///   - background: The background color (default: none). See also [QColor::fromString].
    ///   - foreground: (default: window text color from system palette)
    ///   - text: (default: none) The text to display
    ///   - fontscalar: (default: 1) Scalar for the default font size which is the pixmap height.
    ///
    ///   Examples
    ///
    ///       /absolute/path/to/a/local/image/file.png
    ///       file:/absolute/path/to/a/local/image/file.png
    ///       :path-to-a-qresource-file
    ///       qrc:path-to-a-qresource-file
    ///       qfip:/path/to/any/file/for/example/a.pdf
    ///       qsp:SP_TrashIcon
    ///       xdg:some-themed-icon-name
    ///       gen:?background=blue&foreground=red&text=Hi&fontscalar=0.5
    ///
    /// \returns The pixmap, if available, null pixmap otherwise.
    ///
    /// [resource collection]: https://doc.qt.io/qt-6/resources.html
    /// [QStyle::StandardPixmap enum]: https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
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

