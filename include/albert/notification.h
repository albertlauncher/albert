// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>

namespace albert
{

///
/// The notification class.
///
/// This is basically a wrapper around the QNotification class.
/// @see https://github.com/QtCommunity/QNotification
///
/// \ingroup util_system
///
class ALBERT_EXPORT Notification final : public QObject
{
    Q_OBJECT

public:

    ///
    /// Constructs a notification with the given _title_ and _text_.
    ///
    Notification(const QString &title = {},
                 const QString &text = {},
                 QObject *parent = nullptr);

    ///
    /// Destructs the notification.
    ///
    ~Notification();

    ///
    /// Returns the title of the notification.
    ///
    const QString &title() const;

    ///
    /// Sets the title of the notification to _title_.
    ///
    void setTitle(const QString &title);

    ///
    /// Returns the text of the notification.
    ///
    const QString &text() const;

    ///
    /// Sets the text of the notification to _text_.
    ///
    void setText(const QString &text);

    ///
    /// Send the notification to the notification server.
    ///
    /// This will add the notification to the notification server
    /// and present it to the user (Subject to the users settings).
    ///
    void send();

    ///
    /// Dismiss the notification.
    ///
    /// This will remove the notification from the notification server.
    ///
    void dismiss();

signals:

    ///
    /// Emitted when the notification is activated, i.e. the user clicked on the notification.
    ///
    void activated();

private:

    class ALBERT_NO_EXPORT Private;
    std::unique_ptr<Private> d;
    friend class ALBERT_NO_EXPORT QNotificationManager;

};

}
