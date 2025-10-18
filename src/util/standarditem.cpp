// Copyright (c) 2022-2025 Manuel Schneider

#include "icon.h"
#include "standarditem.h"
using namespace albert;
using namespace std;

StandardItem::~StandardItem() {}

void StandardItem::setId(QString id) { id_ = ::move(id); }

void StandardItem::setText(QString text) { text_ = ::move(text); }

void StandardItem::setSubtext(QString subtext) { subtext_ = ::move(subtext); }

void StandardItem::setIconFactory(function<unique_ptr<Icon>()> icon_factory) { icon_factory_ = ::move(icon_factory); }

std::function<std::unique_ptr<Icon>()> StandardItem::iconFactory() { return icon_factory_; }

void StandardItem::setInputActionText(QString t) { input_action_text_ = ::move(t); }

void StandardItem::setActions(vector<Action> actions) { actions_ = ::move(actions); }

QString StandardItem::id() const { return id_; }

QString StandardItem::text() const { return text_; }

QString StandardItem::subtext() const { return subtext_; }

std::unique_ptr<Icon> StandardItem::icon() const
{
    if (icon_factory_)
        if (auto icon = icon_factory_(); icon)
            return icon;
    return {};
}

QString StandardItem::inputActionText() const { return input_action_text_.isNull() ? text_ : input_action_text_; }

vector<Action> StandardItem::actions() const { return actions_; }
