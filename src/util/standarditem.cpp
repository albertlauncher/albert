// Copyright (c) 2022-2024 Manuel Schneider

#include "standarditem.h"
using namespace albert;
using namespace std;
using namespace util;

StandardItem::StandardItem(QString id,
                           QString text,
                           QString subtext,
                           QString input_action_text,
                           QStringList icon_urls,
                           vector<Action> actions) noexcept :
    id_(::move(id)),
    text_(::move(text)),
    subtext_(::move(subtext)),
    input_action_text_(::move(input_action_text)),
    icon_urls_(::move(icon_urls)),
    actions_(::move(actions))
{}

void StandardItem::setId(QString id) { id_ = ::move(id); }

void StandardItem::setText(QString text) { text_ = ::move(text); }

void StandardItem::setSubtext(QString subtext) { subtext_ = ::move(subtext); }

void StandardItem::setInputActionText(QString t) { input_action_text_ = ::move(t); }

void StandardItem::setIconUrls(QStringList icon_urls) { icon_urls_ = ::move(icon_urls); }

void StandardItem::setActions(vector<Action> actions) { actions_ = ::move(actions); }

QString StandardItem::id() const { return id_; }
QString StandardItem::text() const { return text_; }
QString StandardItem::subtext() const { return subtext_; }
QString StandardItem::inputActionText() const { return input_action_text_; }
QStringList StandardItem::iconUrls() const { return icon_urls_; }
vector<Action> StandardItem::actions() const { return actions_; }

std::shared_ptr<StandardItem> StandardItem::make(QString id,
                                                 QString text,
                                                 QString subtext,
                                                 QString input_action_text,
                                                 QStringList icon_urls,
                                                 std::vector<Action> actions)
{
    return std::make_shared<StandardItem>(::move(id),
                                          ::move(text),
                                          ::move(subtext),
                                          ::move(input_action_text),
                                          ::move(icon_urls),
                                          ::move(actions));
}

std::shared_ptr<StandardItem> StandardItem::make(QString id,
                                                 QString text,
                                                 QString subtext,
                                                 QStringList icon_urls,
                                                 std::vector<Action> actions)
{
    return make_shared<StandardItem>(::move(id),
                                          ::move(text),
                                          ::move(subtext),
                                          QString(),
                                          ::move(icon_urls),
                                          ::move(actions));
}
