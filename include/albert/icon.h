// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <memory>
#include <filesystem>
class QBrush;
class QIcon;
class QPainter;
class QPixmap;
class QRect;
class QSize;
class QString;

namespace albert
{

///
/// Abstract icon engine.
///
/// \ingroup core_query
///
class ALBERT_EXPORT Icon
{
public:

    /// Destructs the icon.
    virtual ~Icon();

    /// Returns a clone of this icon.
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

    /// Uses the given _painter_ to paint the icon into the rectangle _rect_.
    virtual void paint(QPainter *painter, const QRect &rect) = 0;

    ///
    /// Returns `true` if the icon is valid; otherwise returns `false`.
    ///
    /// The base implementation returns `false`.
    ///
    virtual bool isNull();

    /// Returns a URL representation of the icon.
    virtual QString toUrl() const = 0;

    ///
    /// Returns the cache key of the icon.
    ///
    /// The base implementation calls \ref toUrl. Reimplement to get faster lookups.
    ///
    virtual QString cacheKey();

    /// Returns a `QIcon` using _icon_ as icon engine.
    static QIcon qIcon(std::unique_ptr<albert::Icon> icon);

    /// Returns a built-in icon for the given _url_.
    static std::unique_ptr<Icon> iconFromUrl(const QString &url);

    /// Returns a built-in icon for the given _urls_.
    static std::unique_ptr<Icon> iconFromUrls(const QStringList &urls);

    /// @name Image icon
    /// @{

    /// Returns an icon from an image file at _path_.
    static std::unique_ptr<Icon> image(const QString &path);

    /// @copydoc image(const QString &)
    static std::unique_ptr<Icon> image(const std::filesystem::path &path);

    /// @}

    /// @name File type icon
    /// @{

    /// Returns an icon representing the file type of the file at _path_.
    static std::unique_ptr<Icon> fileType(const QString &path);

    /// @copydoc fileType(const QString &)
    static std::unique_ptr<Icon> fileType(const std::filesystem::path &path);

    /// @}

    /// @name Theme icon ([XDG icon lookup](https://specifications.freedesktop.org/icon-theme/latest/))
    /// @{

    /// Returns an icon from the current icon theme with the given _icon_name_.
    static std::unique_ptr<Icon> theme(const QString &icon_name);

    /// @}

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
    /// Returns a standard icon for the given _type_.
    static std::unique_ptr<Icon> standard(StandardIconType type);
    /// @}

    /// @name Grapheme icon
    /// @{

    /// Returns the window text color of the current application palette.
    static QBrush graphemeDefaultBrush();

    /// Returns an icon rendering the given _grapheme_ with \ref graphemeDefaultBrush, scaled by _scalar_.
    static std::unique_ptr<Icon> grapheme(const QString &grapheme, double scalar = 1.0);

    /// Returns an icon rendering the given _grapheme_, scaled by _scalar_ and colored with _brush_.
    static std::unique_ptr<Icon> grapheme(const QString &grapheme,
                                          double scalar,
                                          const QBrush &brush);

    /// @}

    /// @name Iconified icon
    /// @{

    /// Returns the default background brush (a top down gradient from white to some darker white).
    static const QBrush &iconifiedDefaultBackgroundBrush();

    /// Returns the default border color (a gradient slightly darker than the default background).
    static const QBrush &iconifiedDefaultBorderBrush();

    ///
    /// Returns iconified _icon_. i.e. drawn in a colored rounded rectangle with a border.
    ///
    /// _color_ specifies the background color, _border_width_ the border width in device independent pixels,
    /// _border_radius_ the relative border radius (0.0 - 1.0), and _border_color_ the border color.
    ///
    static std::unique_ptr<Icon> iconified(
        std::unique_ptr<Icon> icon,
        const QBrush &background_brush = iconifiedDefaultBackgroundBrush(),
        double border_radius = 1.0,
        int border_width = 1,
        const QBrush &border_color = iconifiedDefaultBorderBrush());

    /// @}

    /// @name Composed icon
    /// @{

    ///
    /// Returns a composed icon from _icon1_ and _icon2_.
    ///
    /// _size1_ and _size2_ specify the relative sizes (0.0 - 1.0) of the icons.
    /// _x1_, _y1_, _x2_, and _y2_ specify the relative positions (0.0 - 1.0, 0.5 is centered) of the icons.
    ///
    static std::unique_ptr<Icon> composed(std::unique_ptr<Icon> icon1,
                                          std::unique_ptr<Icon> icon2,
                                          double size1 = 0.7,
                                          double size2 = 0.7,
                                          double x1 = 0.0,
                                          double y1 = 0.0,
                                          double x2 = 1.0,
                                          double y2 = 1.0);

    /// @}

};

}  // namespace albert
