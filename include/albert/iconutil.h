// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <albert/icon.h>
#include <filesystem>
#include <QString>
class QIcon;
class QBrush;

namespace albert
{

/// @name Icon utilities
///@{

/// Returns a QIcon using _icon_ as icon engine.
ALBERT_EXPORT QIcon qIcon(std::unique_ptr<albert::Icon> icon);

/// Returns a built-in icon for the given _url_.
/// See also \ref Icon::toUrl.
ALBERT_EXPORT std::unique_ptr<Icon> iconFromUrl(const QString &url);

/// Returns a built-in icon for the given _urls_.
/// See also \ref Icon::toUrl.
ALBERT_EXPORT std::unique_ptr<Icon> iconFromUrls(const QStringList &urls);


// ---------------------------------------------------------------------------------------------------------------------

/// Returns an icon from an image file at _path_.
ALBERT_EXPORT std::unique_ptr<Icon> makeImageIcon(const QString &path);

/// @copydoc makeImageIcon
ALBERT_EXPORT std::unique_ptr<Icon> makeImageIcon(const std::filesystem::path &path);


// ---------------------------------------------------------------------------------------------------------------------

/// Returns an icon representing the file type of the file at _path_.
ALBERT_EXPORT std::unique_ptr<Icon> makeFileTypeIcon(const QString &path);

/// @copydoc makeFileTypeIcon
ALBERT_EXPORT std::unique_ptr<Icon> makeFileTypeIcon(const std::filesystem::path &path);


// ---------------------------------------------------------------------------------------------------------------------

/// This enum describes the available standard icons.
/// See [Qt documentation](https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum) for more details.
enum StandardIconType
{
    TitleBarMenuButton,
    TitleBarMinButton,
    TitleBarMaxButton,
    TitleBarCloseButton,
    TitleBarNormalButton,
    TitleBarShadeButton,
    TitleBarUnshadeButton,
    TitleBarContextHelpButton,
    DockWidgetCloseButton,
    MessageBoxInformation,
    MessageBoxWarning,
    MessageBoxCritical,
    MessageBoxQuestion,
    DesktopIcon,
    TrashIcon,
    ComputerIcon,
    DriveFDIcon,
    DriveHDIcon,
    DriveCDIcon,
    DriveDVDIcon,
    DriveNetIcon,
    DirOpenIcon,
    DirClosedIcon,
    DirLinkIcon,
    DirLinkOpenIcon,
    FileIcon,
    FileLinkIcon,
    ToolBarHorizontalExtensionButton,
    ToolBarVerticalExtensionButton,
    FileDialogStart,
    FileDialogEnd,
    FileDialogToParent,
    FileDialogNewFolder,
    FileDialogDetailedView,
    FileDialogInfoView,
    FileDialogContentsView,
    FileDialogListView,
    FileDialogBack,
    DirIcon,
    DialogOkButton,
    DialogCancelButton,
    DialogHelpButton,
    DialogOpenButton,
    DialogSaveButton,
    DialogCloseButton,
    DialogApplyButton,
    DialogResetButton,
    DialogDiscardButton,
    DialogYesButton,
    DialogNoButton,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    ArrowBack,
    ArrowForward,
    DirHomeIcon,
    CommandLink,
    VistaShield,
    BrowserReload,
    BrowserStop,
    MediaPlay,
    MediaStop,
    MediaPause,
    MediaSkipForward,
    MediaSkipBackward,
    MediaSeekForward,
    MediaSeekBackward,
    MediaVolume,
    MediaVolumeMuted,
    LineEditClearButton,
    DialogYesToAllButton,
    DialogNoToAllButton,
    DialogSaveAllButton,
    DialogAbortButton,
    DialogRetryButton,
    DialogIgnoreButton,
    RestoreDefaultsButton,
    TabCloseButton
};

/// Returns a standard icon for the given _type_.
ALBERT_EXPORT std::unique_ptr<Icon> makeStandardIcon(StandardIconType type);


// ---------------------------------------------------------------------------------------------------------------------

/// Returns an icon from the current icon theme with the given _icon_name_.
ALBERT_EXPORT std::unique_ptr<Icon> makeThemeIcon(const QString &icon_name);


// ---------------------------------------------------------------------------------------------------------------------

/// Returns the default color (black).
ALBERT_EXPORT const QBrush &graphemeIconDefaultColor();

/// Returns the default scaling factor (1.0).
ALBERT_EXPORT double graphemeIconDefaultScalar();

/// Returns an icon rendering the given _grapheme_, scaled by _scalar_ and colored with _color_.
ALBERT_EXPORT std::unique_ptr<Icon> makeGraphemeIcon(const QString &grapheme,
                                                     double scalar = graphemeIconDefaultScalar(),
                                                     const QBrush &color = graphemeIconDefaultColor());


// ---------------------------------------------------------------------------------------------------------------------

/// Returns the default color (black).
ALBERT_EXPORT const QBrush &rectIconDefaultColor();

/// Returns the default border color (black).
ALBERT_EXPORT const QBrush &rectIconDefaultBorderColor();

/// Returns the default border radius (1.0).
ALBERT_EXPORT double rectIconDefaultRadius();

/// Returns the default border width (0).
ALBERT_EXPORT int rectIconDefaultBorderWidth();

/// Returns a simple rectangular icon with the given _color_, _radius_, _border_width_ and _border_color_.
ALBERT_EXPORT std::unique_ptr<Icon> makeRectIcon(const QBrush &color = rectIconDefaultColor(),
                                                 double radius = rectIconDefaultRadius(),
                                                 int border_width = rectIconDefaultBorderWidth(),
                                                 const QBrush &border_color = rectIconDefaultBorderColor());


// ---------------------------------------------------------------------------------------------------------------------

/// Returns the default background color (a top down gradient from white to some darker white).
ALBERT_EXPORT const QBrush &iconifiedIconDefaultColor();

/// Returns the default relative radius 1.0.
ALBERT_EXPORT double iconifiedIconDefaultBorderRadius();

/// Returns the default border width (1).
ALBERT_EXPORT int iconifiedIconDefaultBorderWidth();

/// Returns the default border color (a gradient slightly darker than the default background).
ALBERT_EXPORT const QBrush &iconifiedIconDefaultBorderColor();


/// Returns an iconified _src_. i.e. drawn in a colored rounded rectangle with a border.
/// _color_ specifies the background color, _border_width_ the border width in device independent pixels,
/// _border_radius_ the relative border radius (0.0 - 1.0), and _border_color_ the border color.
ALBERT_EXPORT std::unique_ptr<Icon> makeIconifiedIcon(std::unique_ptr<Icon> src,
                                                      const QBrush &color = iconifiedIconDefaultColor(),
                                                      double border_radius = iconifiedIconDefaultBorderRadius(),
                                                      int border_width = iconifiedIconDefaultBorderWidth(),
                                                      const QBrush &border_color = iconifiedIconDefaultBorderColor());


// ---------------------------------------------------------------------------------------------------------------------

/// Returns the default relative size 0.7.
ALBERT_EXPORT double composedIconDefaultSize();

/// Returns the default relative position 0.0 of the first item.
ALBERT_EXPORT double composedIconDefaultPos1();

/// Returns the default relative position 1.0 of the second item.
ALBERT_EXPORT double composedIconDefaultPos2();

/// Returns a composed icon of _src1_ and _src2_.
/// _size1_ and _size2_ specify the relative sizes (0.0 - 1.0) of the icons.
/// _x1_, _y1_, _x2_, and _y2_ specify the relative positions (0.0 - 1.0, 0.5 is centered) of the icons.
ALBERT_EXPORT std::unique_ptr<Icon> makeComposedIcon(std::unique_ptr<Icon> src1,
                                                     std::unique_ptr<Icon> src2,
                                                     double size1 = composedIconDefaultSize(),
                                                     double size2 = composedIconDefaultSize(),
                                                     double x1 = composedIconDefaultPos1(),
                                                     double y1 = composedIconDefaultPos1(),
                                                     double x2 = composedIconDefaultPos2(),
                                                     double y2 = composedIconDefaultPos2());
///@}

}  // namespace albert


