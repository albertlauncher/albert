// Copyright (c) 2024 Manuel Schneider

#include "albert/util/notification.h"
#include <QNotification>
using namespace albert;

class Notification::Private
{
public:
    QNotification notification;
};

Notification::Notification(const QString &title, const QString &text, QObject *parent)
    : QObject(parent), d(new Private{{title, text}})
{
    connect(&d->notification, &QNotification::activated,
            this, &Notification::activated);
}

Notification::~Notification() = default;

const QString &Notification::title() const
{
    return d->notification.title();
}

void Notification::setTitle(const QString &title)
{
    d->notification.setTitle(title);
}

const QString &Notification::text() const
{
    return d->notification.text();
}

void Notification::setText(const QString &text)
{
    d->notification.setText(text);
}

void Notification::send()
{
    d->notification.send();
}

void Notification::dismiss()
{
    d->notification.dismiss();
}
