// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
#include <albert/extension.h>
class QUrl;

namespace albert
{
class ALBERT_EXPORT UrlHandler : virtual public Extension
{
public:

    virtual void handle(const QUrl&) = 0;

protected:

    ~UrlHandler() override;
};
}
