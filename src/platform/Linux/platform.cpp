// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/util/notification.h"
#include "platform/platform.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
using namespace albert;

void platform::initPlatform(){}

void platform::initNativeWindow(unsigned long long){}

class NotificationPrivate
{
public:
    NotificationPrivate() :
        interface(QStringLiteral("org.freedesktop.Notifications"),
                  QStringLiteral("/org/freedesktop/Notifications"),
                  QStringLiteral("org.freedesktop.Notifications")){
        if(!interface.isValid())
            CRIT << interface.lastError();
    }

    uint notification_id;
    QDBusInterface interface;
};

albert::Notification::Notification(const QString &title, const QString &body) : d(new NotificationPrivate)
{
    QDBusReply<uint> reply = d->interface.call(
        QStringLiteral("Notify"),
        "Albert",
        0u,
        "albert",
        title,
        body,
        QStringList(),
        QVariantMap(),
        0
    );

    if(reply.isValid())
        d->notification_id = reply.value();
    else
        WARN << reply.error();
}

albert::Notification::~Notification()
{
    QDBusReply<void> reply = d->interface.call(QStringLiteral("CloseNotification"), d->notification_id);
    if(!reply.isValid())
        WARN << reply.error();
}














