// Copyright (c) 2026-2026 Manuel Schneider

#include "app.h"
#include "logging.h"
#include "urldispatcher.h"
#include "urlhandler.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
using namespace albert;
using namespace std;

UrlDispatcher::UrlDispatcher() { QDesktopServices::setUrlHandler("albert", this, "dispatch"); }

UrlDispatcher::~UrlDispatcher() { QDesktopServices::unsetUrlHandler("albert"); }

void UrlDispatcher::dispatch(const QUrl &url)
{
    DEBG << "Handle url" << url.toString();

    if (url.scheme() == qApp->applicationName())
    {
        if (url.authority().isEmpty())
        {
           // ?
        }
        else if (auto h = app().extension<UrlHandler>(url.authority()); h)
            h->handle(url);
        else
            WARN << "URL handler not available: " + url.authority().toLocal8Bit();
    }
    else
        WARN << "Invalid URL scheme" << url.scheme();
}
