// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QStringList>
#include <albert/export.h>

namespace albert::util
{

///
/// URL based icon factory.
///
/// Supported URL schemes:
///
/// - `<path>` or
/// - `file:<path>` Use the file at path as icon.
/// - `:<path>` or
/// - `qrc:<path>` Use the file at path in the [resource collection] as icon.
/// - `qfip:<path>` Uses fileIcon(const QString &path)
/// - `qsp:<pixmap enumerator>` Get an icon from [QStyle::StandardPixmap] enum.
/// - `xdg:<icon name>` Uses xdgIconLookup(const QString &name);
/// - `gen:<>` Uses genericPixmapFactory. See also [QColor::fromString].
///
/// Examples
///
///     /absolute/path/to/a/local/image/file.png
///     file:/absolute/path/to/a/local/image/file.png
///     :path-to-a-qresource-file
///     qrc:path-to-a-qresource-file
///     qfip:/path/to/any/file/for/example/a.pdf
///     qsp:SP_TrashIcon
///     xdg:some-themed-icon-name
///     gen:?background=blue&foreground=red&text=Hi&fontscalar=0.5
///
/// \param url The icon URL.
/// \param requestedSize The size the pixmap should have if possible.
/// \returns The pixmap, if available, null pixmap otherwise. The size can be smaller
///         than requestedSize, but is never larger.
///
/// [resource collection]: https://doc.qt.io/qt-6/resources.html
/// [QStyle::StandardPixmap]: https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum
/// [QColor::fromString]: https://doc.qt.io/qt/qcolor.html#fromString
///
QPixmap ALBERT_EXPORT pixmapFromUrl(const QString &url, const QSize &requestedSize);

///
/// URL list based icon factory.
///
/// See pixmapFromUrl(const QString &url, QSize *size, const QSize &requestedSize).
///
/// \param urls The icon URLs.
/// \param requestedSize The size the pixmap should have if possible.
/// \returns The first pixmap available in the urls list. Null pixmap otherwise.
///
QPixmap ALBERT_EXPORT pixmapFromUrls(const QStringList &urls, const QSize &requestedSize);

///
/// URL based icon factory.
///
/// \copydetails pixmapFromUrl(const QString &url, QSize *size, const QSize &requestedSize)
///
/// \param url The icon URL.
/// \returns The icon, if available, null icon otherwise.
///
QIcon ALBERT_EXPORT iconFromUrl(const QString &url);

///
/// URL list based icon factory.
///
/// See iconFromUrl(const QString &url).
///
/// \param urls The icon URLs.
/// \returns The first icon available in the urls list. Null icon otherwise.
///
QIcon ALBERT_EXPORT iconFromUrls(const QStringList &urls);

///
/// Generic pixmap factory.
///
/// Supports drawing a background circle with some text on it.
///
/// \param bgcolor The background color. Default: none.
/// \param fgcolor The text color. Default: black.
/// \param text The text to display. Default: empty.
/// \param scalar Scalar for the default font size which is the pixmap height. Default: 1.
/// \returns The generic icon.
///
QPixmap ALBERT_EXPORT genericPixmap(int size, const QColor& bgcolor = {}, const QColor& fgcolor = Qt::black, const QString& text = {}, float scalar = 1.);

///
/// Generic icon factory.
///
/// Supports drawing a background circle with some text on it.
///
/// \param bgcolor The background color. Default: none.
/// \param fgcolor The text color. Default: black.
/// \param text The text to display. Default: empty.
/// \param scalar Scalar for the default font size which is the pixmap height. Default: 1.
/// \returns The generic pixmap.
///
QIcon ALBERT_EXPORT genericIcon(const QColor& bgcolor = {}, const QColor& fgcolor = Qt::black, const QString& text = {}, float scalar = 1.);

///
/// Create an icon for a file using [QFileIconProvider].
///
/// [QFileIconProvider]: https://doc.qt.io/qt/qfileiconprovider.html
///
/// \param path The path to the file.
/// \returns The file icon.
///
QIcon ALBERT_EXPORT fileIcon(const QString &path);

///
/// Performs an icon lookup according to the [freedesktop icon theme specification].
///
/// Available only on platforms supporting it.
///
/// [freedesktop icon theme specification]: https://specifications.freedesktop.org/icon-theme-spec/latest/
///
/// \param name The icon name.
/// \returns The path if available, null string otherwise.
///
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
QString ALBERT_EXPORT xdgIconLookup(const QString &name);
#endif

}

