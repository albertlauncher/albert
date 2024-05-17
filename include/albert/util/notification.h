// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>

namespace albert
{

///
/// The notification class.
///
/// This is basically a wrapper around the QNotification class.
/// @see https://github.com/QtCommunity/QNotification
///
class ALBERT_EXPORT Notification final : public QObject
{
    Q_OBJECT

public:

    Notification(const QString &title = {},
                 const QString &text = {},
                 QObject *parent = nullptr);
    ~Notification();

    /// The title of the notification.
    /// @return @copybrief title
    const QString &title() const;

    /// Set the title of the notification.
    /// @param title @copybrief title
    void setTitle(const QString &title);

    /// The text of the notification.
    /// @return @copybrief text
    const QString &text() const;

    /// Set the text of the notification.
    /// @param text @copybrief text
    void setText(const QString &text);

    /// Send the notification to the notification server.
    /// This will add the notification to the notification server
    /// and present it to the user (Subject to the users settings).
    void send();

    /// Dismiss the notification.
    /// This will remove the notification from the notification server.
    void dismiss();

signals:

    /// Emitted when the notification is activated.
    /// I.e. the user clicked on the notification.
    void activated();

private:

    class ALBERT_NO_EXPORT Private;
    ALBERT_NO_EXPORT std::unique_ptr<Private> d;
    friend class ALBERT_NO_EXPORT QNotificationManager;

};

}
