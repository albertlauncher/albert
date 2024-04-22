// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/util/notification.h"
#include <UserNotifications/UserNotifications.h>
using namespace albert;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

class NotificationPrivate
{
public:
    NSUserNotification *notification;
};

albert::Notification::Notification(const QString &title, const QString &body) : d(new NotificationPrivate)
{
    d->notification = [[NSUserNotification alloc] init];
    d->notification.title = title.toNSString();
//    d->notification.subtitle = body.toNSString();
    d->notification.informativeText = body.toNSString();
    d->notification.hasActionButton = NO;
    d->notification.soundName = NSUserNotificationDefaultSoundName;
    [NSUserNotificationCenter.defaultUserNotificationCenter deliverNotification:d->notification];
}

albert::Notification::~Notification()
{
    [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:d->notification];
    [d->notification release];
    delete d;
}

#pragma clang diagnostic pop
























