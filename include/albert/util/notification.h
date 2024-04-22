// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
class NotificationPrivate;

namespace albert
{

///
/// A system tray notification.
/// The notification is visible as long as this object exists.
///
class ALBERT_EXPORT Notification
{
public:
    Notification(const QString &title={}, const QString &body={});
    ~Notification();

private:
    NotificationPrivate *d;
};

}
