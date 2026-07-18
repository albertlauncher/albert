// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <QObject>
class QUrl;

class UrlDispatcher : public QObject
{
    Q_OBJECT
public:
    UrlDispatcher();
    ~UrlDispatcher();
    Q_INVOKABLE void dispatch(const QUrl &url);
};
