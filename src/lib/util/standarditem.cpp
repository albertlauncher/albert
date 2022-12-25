// Copyright (c) 2022 Manuel Schneider

#include "albert/util/standarditem.h"
using namespace albert;

StandardItem::StandardItem(QString id,
                           QString text,
                           QString subtext,
                           QString input_action_text,
                           QStringList icon_urls,
                           Actions actions) :
        id_(std::move(id)),
        text_(std::move(text)),
        subtext_(std::move(subtext)),
        input_action_text_(std::move(input_action_text)),
        icon_urls_(std::move(std::move(icon_urls))),
        actions_(std::move(actions)) {}

void StandardItem::setId(QString id) { id_ = std::move(id); }
void StandardItem::setText(QString text) { text_ = std::move(text); }
void StandardItem::setSubtext(QString subtext) { subtext_ = std::move(subtext); }
void StandardItem::setInputActionText(QString input_action_text) { input_action_text_ = std::move(input_action_text); }
void StandardItem::setIconUrls(QStringList icon_urls) { icon_urls_ = std::move(icon_urls); }
void StandardItem::setActions(Actions actions) { actions_ = std::move(actions); }

QString StandardItem::id() const { return id_; }
QString StandardItem::text() const { return text_; }
QString StandardItem::subtext() const { return subtext_; }
QString StandardItem::inputActionText() const { return input_action_text_; }
QStringList StandardItem::iconUrls() const { return icon_urls_; }
bool StandardItem::hasActions() const { return !actions_.empty(); }
std::vector<Action> StandardItem::actions() const { return actions_; }


std::shared_ptr<StandardItem> StandardItem::make(QString id,
                                                 QString text,
                                                 QString subtext,
                                                 QString input_action_text,
                                                 QStringList icon_urls,
                                                 Actions actions)
{
    return std::make_shared<StandardItem>(std::move(id),
                                          std::move(text),
                                          std::move(subtext),
                                          std::move(input_action_text),
                                          std::move(std::move(icon_urls)),
                                          std::move(actions));
}

std::shared_ptr<StandardItem> StandardItem::make(QString id,
                                                 QString text,
                                                 QString subtext,
                                                 QStringList icon_urls,
                                                 Actions actions)
{
    return std::make_shared<StandardItem>(std::move(id),
                                          std::move(text),
                                          std::move(subtext),
                                          QString(),
                                          std::move(std::move(icon_urls)),
                                          std::move(actions));
}
