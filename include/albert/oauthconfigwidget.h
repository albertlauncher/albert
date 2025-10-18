// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <QWidget>
#include <albert/export.h>

namespace albert
{
class OAuth2;

///
/// Ready to use OAuth login widget.
///
/// \ingroup util_ui
///
class ALBERT_EXPORT OAuthConfigWidget : public QWidget
{
public:

    ///
    /// Constructs an \ref OAuthConfigWidget for _oauth_.
    ///
    OAuthConfigWidget(OAuth2 &oauth);

    ///
    /// Destructs the \ref OAuthConfigWidget.
    ///
    ~OAuthConfigWidget();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
