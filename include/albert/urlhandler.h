// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
#include <albert/extension.h>
class QUrl;

namespace albert
{

/// Albert scheme URL handler interface.
///
/// Use this interface to register `albert:` URL handlers based on \ref Extension::id.
/// URLs with the host matching this extension's id are passed to the \ref handle() method.
/// E.g. the URL `albert://github/?…` will be redirected to the GitHub extension.
class ALBERT_EXPORT UrlHandler : virtual public Extension
{
public:

    /// Handles the _url_ received.
    virtual void handle(const QUrl &url) = 0;

protected:

    ~UrlHandler() override;
};
}
