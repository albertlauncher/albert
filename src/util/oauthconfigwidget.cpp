// SPDX-FileCopyrightText: 2025 Manuel Schneider

#include "oauth.h"
#include "oauthconfigwidget.h"
#include "widgetsutil.h"
#include <QCoreApplication>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
using namespace albert::util;
using namespace std;

class OAuthConfigWidget::Private
{
public:
    OAuthConfigWidget *q;
    albert::util::OAuth2 &oauth;
    QFormLayout *formLayout;
    QLabel *label_client_id;
    QLabel *label_client_secret;
    QLabel *label_auth;
    QLabel *label_auth_state;
    QLineEdit *lineEdit_client_id;
    QLineEdit *lineEdit_client_secret;
    QPushButton *pushButton_auth;

    Private(OAuthConfigWidget *_q, albert::util::OAuth2 &_oauth):
        q(_q), oauth(_oauth)
    {
        formLayout = new QFormLayout;
        label_client_id = new QLabel;
        label_client_secret = new QLabel;
        label_auth = new QLabel;
        label_auth_state = new QLabel;
        lineEdit_client_id = new QLineEdit;
        lineEdit_client_secret = new QLineEdit;
        pushButton_auth = new QPushButton;

        label_client_id->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
        label_client_secret->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_auth->sizePolicy().hasHeightForWidth());
        pushButton_auth->setSizePolicy(sizePolicy);

        formLayout->addRow(label_client_id, lineEdit_client_id);
        if (!oauth.isPkceEnabled())
            formLayout->addRow(label_client_secret, lineEdit_client_secret);
        formLayout->addRow(label_auth, label_auth_state);
        formLayout->addRow(nullptr, pushButton_auth);

        label_client_id->setText(QCoreApplication::translate("OAuthConfigWidget", "Client identifier", nullptr));
        label_client_secret->setText(QCoreApplication::translate("OAuthConfigWidget", "Client secret", nullptr));
        label_auth->setText(QCoreApplication::translate("OAuthConfigWidget", "Authorization", nullptr));
        pushButton_auth->setText(QCoreApplication::translate("OAuthConfigWidget", "Request", "action"));

        updateGrantState();

        connect(&oauth, &OAuth2::stateChanged,
                q, [this]{ updateGrantState(); });

        connect(pushButton_auth, &QPushButton::clicked,
                &oauth, &OAuth2::requestAccess);

        bind(lineEdit_client_id,
             &oauth,
             &OAuth2::clientId,
             &OAuth2::setClientId,
             &OAuth2::clientIdChanged);

        bind(lineEdit_client_secret,
             &oauth,
             &OAuth2::clientSecret,
             &OAuth2::setClientSecret,
             &OAuth2::clientSecretChanged);

        q->setLayout(formLayout);
    }

    void updateGrantState()
    {
        using enum OAuth2::State;
        switch (oauth.state())
        {
        case Awaiting:
            label_auth_state->setText(QCoreApplication::translate("OAuthConfigWidget",
                                                                  "Awaiting authorization…", nullptr));
            pushButton_auth->show();
            break;
        case NotAuthorized:
            if (const auto e = oauth.error(); e.isEmpty())
                label_auth_state->setText(QCoreApplication::translate("OAuthConfigWidget",
                                                                      "Not authorized.", nullptr));
            else
                label_auth_state->setText(e);
            pushButton_auth->show();
            break;
        case Granted:
            label_auth_state->setText(QStringLiteral("<font color=\"green\">%1</font>")
                                          .arg(QCoreApplication::translate("OAuthConfigWidget",
                                                                           "Granted", nullptr)));
            // pushButton_auth->hide();
            q->window()->show();
            q->window()->raise();
            q->window()->activateWindow();
        default:
            break;
        }
    }
};

OAuthConfigWidget::OAuthConfigWidget(OAuth2 &_oauth):
    QWidget(nullptr),
    d(make_unique<Private>(this, _oauth)) {}

OAuthConfigWidget::~OAuthConfigWidget() {}
