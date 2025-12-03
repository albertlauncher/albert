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

/// \addtogroup util_icon
/// @{

///
/// Returns a QIcon using _icon_ as icon engine.
///
ALBERT_EXPORT QIcon qIcon(std::unique_ptr<albert::Icon> icon);

///
/// Returns a built-in icon for the given _url_.
///
/// See also \ref Icon::toUrl.
///
ALBERT_EXPORT std::unique_ptr<Icon> iconFromUrl(const QString &url);

///
/// Returns a built-in icon for the given _urls_.
///
/// See also \ref Icon::toUrl.
///
ALBERT_EXPORT std::unique_ptr<Icon> iconFromUrls(const QStringList &urls);


// ---------------------------------------------------------------------------------------------------------------------

/// @name Image icon
/// @{

///
/// Returns an icon from an image file at _path_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeImageIcon(const QString &path);

///
/// @copydoc makeImageIcon
///
ALBERT_EXPORT std::unique_ptr<Icon> makeImageIcon(const std::filesystem::path &path);

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name File type icon
/// @{

///
/// Returns an icon representing the file type of the file at _path_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeFileTypeIcon(const QString &path);

///
/// @copydoc makeFileTypeIcon
///
ALBERT_EXPORT std::unique_ptr<Icon> makeFileTypeIcon(const std::filesystem::path &path);

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Standard icon (QStyle standard pixmap)
/// @{

///
/// This enum describes the available standard icons.
///
/// See [Qt documentation](https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum) for more details.
///
enum StandardIconType
{
    TitleBarMinButton = 1,                 ///< Menu button on a title bar.
    TitleBarMenuButton = 0,                ///< Minimize button on title bars (e.g., in QMdiSubWindow).
    TitleBarMaxButton = 2,                 ///< Maximize button on title bars.
    TitleBarCloseButton = 3,               ///< Close button on title bars.
    TitleBarNormalButton = 4,              ///< Normal (restore) button on title bars.
    TitleBarShadeButton = 5,               ///< Shade button on title bars.
    TitleBarUnshadeButton = 6,             ///< Unshade button on title bars.
    TitleBarContextHelpButton = 7,         ///< The Context help button on title bars.
    MessageBoxInformation = 9,             ///< The "information" icon.
    MessageBoxWarning = 10,                ///< The "warning" icon.
    MessageBoxCritical = 11,               ///< The "critical" icon.
    MessageBoxQuestion = 12,               ///< The "question" icon.
    DesktopIcon = 13,                      ///< The "desktop" icon.
    TrashIcon = 14,                        ///< The "trash" icon.
    ComputerIcon = 15,                     ///< The "My computer" icon.
    DriveFDIcon = 16,                      ///< The floppy icon.
    DriveHDIcon = 17,                      ///< The harddrive icon.
    DriveCDIcon = 18,                      ///< The CD icon.
    DriveDVDIcon = 19,                     ///< The DVD icon.
    DriveNetIcon = 20,                     ///< The network icon.
    DirHomeIcon = 56,                      ///< The home directory icon.
    DirOpenIcon = 21,                      ///< The open directory icon.
    DirClosedIcon = 22,                    ///< The closed directory icon.
    DirIcon = 38,                          ///< The directory icon.
    DirLinkIcon = 23,                      ///< The link to directory icon.
    DirLinkOpenIcon = 24,                  ///< The link to open directory icon.
    FileIcon = 25,                         ///< The file icon.
    FileLinkIcon = 26,                     ///< The link to file icon.
    FileDialogStart = 29,                  ///< The "start" icon in a file dialog.
    FileDialogEnd = 30,                    ///< The "end" icon in a file dialog.
    FileDialogToParent = 31,               ///< The "parent directory" icon in a file dialog.
    FileDialogNewFolder = 32,              ///< The "create new folder" icon in a file dialog.
    FileDialogDetailedView = 33,           ///< The detailed view icon in a file dialog.
    FileDialogInfoView = 34,               ///< The file info icon in a file dialog.
    FileDialogContentsView = 35,           ///< The contents view icon in a file dialog.
    FileDialogListView = 36,               ///< The list view icon in a file dialog.
    FileDialogBack = 37,                   ///< The back arrow in a file dialog.
    DockWidgetCloseButton = 8,             ///< Close button on dock windows (see also QDockWidget).
    ToolBarHorizontalExtensionButton = 27, ///< Extension button for horizontal toolbars.
    ToolBarVerticalExtensionButton = 28,   ///< Extension button for vertical toolbars.
    DialogOkButton = 39,                   ///< Icon for a standard OK button in a QDialogButtonBox.
    DialogCancelButton = 40,               ///< Icon for a standard Cancel button in a QDialogButtonBox.
    DialogHelpButton = 41,                 ///< Icon for a standard Help button in a QDialogButtonBox.
    DialogOpenButton = 42,                 ///< Icon for a standard Open button in a QDialogButtonBox.
    DialogSaveButton = 43,                 ///< Icon for a standard Save button in a QDialogButtonBox.
    DialogCloseButton = 44,                ///< Icon for a standard Close button in a QDialogButtonBox.
    DialogApplyButton = 45,                ///< Icon for a standard Apply button in a QDialogButtonBox.
    DialogResetButton = 46,                ///< Icon for a standard Reset button in a QDialogButtonBox.
    DialogDiscardButton = 47,              ///< Icon for a standard Discard button in a QDialogButtonBox.
    DialogYesButton = 48,                  ///< Icon for a standard Yes button in a QDialogButtonBox.
    DialogNoButton = 49,                   ///< Icon for a standard No button in a QDialogButtonBox.
    ArrowUp = 50,                          ///< Icon arrow pointing up.
    ArrowDown = 51,                        ///< Icon arrow pointing down.
    ArrowLeft = 52,                        ///< Icon arrow pointing left.
    ArrowRight = 53,                       ///< Icon arrow pointing right.
    ArrowBack = 54,                        ///< Equivalent to SP_ArrowLeft when the current layout direction is Qt::LeftToRight, otherwise SP_ArrowRight.
    ArrowForward = 55,                     ///< Equivalent to SP_ArrowRight when the current layout direction is Qt::LeftToRight, otherwise SP_ArrowLeft.
    CommandLink = 57,                      ///< Icon used to indicate a Vista style command link glyph.
    VistaShield = 58,                      ///< Icon used to indicate UAC prompts on Windows Vista. This will return a null pixmap or icon on all other platforms.
    BrowserReload = 59,                    ///< Icon indicating that the current page should be reloaded.
    BrowserStop = 60,                      ///< Icon indicating that the page loading should stop.
    MediaPlay = 61,                        ///< Icon indicating that media should begin playback.
    MediaStop = 62,                        ///< Icon indicating that media should stop playback.
    MediaPause = 63,                       ///< Icon indicating that media should pause playback.
    MediaSkipForward = 64,                 ///< Icon indicating that media should skip forward.
    MediaSkipBackward = 65,                ///< Icon indicating that media should skip backward.
    MediaSeekForward = 66,                 ///< Icon indicating that media should seek forward.
    MediaSeekBackward = 67,                ///< Icon indicating that media should seek backward.
    MediaVolume = 68,                      ///< Icon indicating a volume control.
    MediaVolumeMuted = 69,                 ///< Icon indicating a muted volume control.
    LineEditClearButton = 70,              ///< Icon for a standard clear button in a QLineEdit.
    DialogYesToAllButton = 71,             ///< Icon for a standard YesToAll button in a QDialogButtonBox.
    DialogNoToAllButton = 72,              ///< Icon for a standard NoToAll button in a QDialogButtonBox.
    DialogSaveAllButton = 73,              ///< Icon for a standard SaveAll button in a QDialogButtonBox.
    DialogAbortButton = 74,                ///< Icon for a standard Abort button in a QDialogButtonBox.
    DialogRetryButton = 75,                ///< Icon for a standard Retry button in a QDialogButtonBox.
    DialogIgnoreButton = 76,               ///< Icon for a standard Ignore button in a QDialogButtonBox.
    RestoreDefaultsButton = 77,            ///< Icon for a standard RestoreDefaults button in a QDialogButtonBox.
    TabCloseButton = 78,                   ///< Icon for the close button in the tab of a QTabBar.
};


///
/// Returns a standard icon for the given _type_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeStandardIcon(StandardIconType type);

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Theme icon (XDG)
/// @{

///
/// Returns an icon from the current icon theme with the given _icon_name_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeThemeIcon(const QString &icon_name);

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Grapheme icon
/// @{

///
/// Returns the default color (black).
///
ALBERT_EXPORT const QBrush &graphemeIconDefaultColor();

///
/// Returns the default scaling factor (1.0).
///
ALBERT_EXPORT double graphemeIconDefaultScalar();

///
/// Returns an icon rendering the given _grapheme_, scaled by _scalar_ and colored with _color_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeGraphemeIcon(const QString &grapheme,
                                                     double scalar = graphemeIconDefaultScalar(),
                                                     const QBrush &color = graphemeIconDefaultColor());

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Rect icon
/// @{

///
/// Returns the default color (black).
///
ALBERT_EXPORT const QBrush &rectIconDefaultColor();

///
/// Returns the default border color (black).
///
ALBERT_EXPORT const QBrush &rectIconDefaultBorderColor();

///
/// Returns the default border radius (1.0).
///
ALBERT_EXPORT double rectIconDefaultRadius();

///
/// Returns the default border width (0).
///
ALBERT_EXPORT int rectIconDefaultBorderWidth();

///
/// Returns a simple rectangular icon with the given _color_, _radius_, _border_width_ and _border_color_.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeRectIcon(const QBrush &color = rectIconDefaultColor(),
                                                 double radius = rectIconDefaultRadius(),
                                                 int border_width = rectIconDefaultBorderWidth(),
                                                 const QBrush &border_color = rectIconDefaultBorderColor());

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Iconified icon
/// @{

///
/// Returns the default background color (a top down gradient from white to some darker white).
///
ALBERT_EXPORT const QBrush &iconifiedIconDefaultColor();

///
/// Returns the default relative radius 1.0.
///
ALBERT_EXPORT double iconifiedIconDefaultBorderRadius();

///
/// Returns the default border width (1).
///
ALBERT_EXPORT int iconifiedIconDefaultBorderWidth();

///
/// Returns the default border color (a gradient slightly darker than the default background).
///
ALBERT_EXPORT const QBrush &iconifiedIconDefaultBorderColor();


///
/// Returns an iconified _src_. i.e. drawn in a colored rounded rectangle with a border.
///
/// _color_ specifies the background color, _border_width_ the border width in device independent pixels,
/// _border_radius_ the relative border radius (0.0 - 1.0), and _border_color_ the border color.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeIconifiedIcon(std::unique_ptr<Icon> src,
                                                      const QBrush &color = iconifiedIconDefaultColor(),
                                                      double border_radius = iconifiedIconDefaultBorderRadius(),
                                                      int border_width = iconifiedIconDefaultBorderWidth(),
                                                      const QBrush &border_color = iconifiedIconDefaultBorderColor());

/// @}

// ---------------------------------------------------------------------------------------------------------------------

/// @name Composed icon
/// @{

///
/// Returns the default relative size 0.7.
///
ALBERT_EXPORT double composedIconDefaultSize();

///
/// Returns the default relative position 0.0 of the first item.
///
ALBERT_EXPORT double composedIconDefaultPos1();

///
/// Returns the default relative position 1.0 of the second item.
///
ALBERT_EXPORT double composedIconDefaultPos2();

///
/// Returns a composed icon of _src1_ and _src2_.
///
/// _size1_ and _size2_ specify the relative sizes (0.0 - 1.0) of the icons.
/// _x1_, _y1_, _x2_, and _y2_ specify the relative positions (0.0 - 1.0, 0.5 is centered) of the icons.
///
ALBERT_EXPORT std::unique_ptr<Icon> makeComposedIcon(std::unique_ptr<Icon> src1,
                                                     std::unique_ptr<Icon> src2,
                                                     double size1 = composedIconDefaultSize(),
                                                     double size2 = composedIconDefaultSize(),
                                                     double x1 = composedIconDefaultPos1(),
                                                     double y1 = composedIconDefaultPos1(),
                                                     double x2 = composedIconDefaultPos2(),
                                                     double y2 = composedIconDefaultPos2());

/// @}

/// @}

}  // namespace albert


