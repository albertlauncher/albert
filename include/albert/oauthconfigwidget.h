// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QString>
#include <QWidget>
#include <albert/export.h>

namespace albert
{
class OAuth2;

class ALBERT_EXPORT OAuthConfigWidget : public QWidget
{
public:

    OAuthConfigWidget(OAuth2 &oauth);
    ~OAuthConfigWidget();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
