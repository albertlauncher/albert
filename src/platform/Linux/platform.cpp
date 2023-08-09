// Copyright (c) 2023 Manuel Schneider

#include "platform/platform.h"
#include "albert/albert.h"
using namespace albert;

void platform::initPlatform(){}

void platform::initNativeWindow(unsigned long long){}

class NotificationPrivate
{
public:
};

albert::Notification::Notification(const QString &title, const QString &subTitle, const QString &text) : d(new NotificationPrivate)
{
}

albert::Notification::~Notification()
{
}














