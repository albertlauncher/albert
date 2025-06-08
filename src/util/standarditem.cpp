// Copyright (c) 2022-2024 Manuel Schneider

#include "standarditem.h"
using namespace albert;
using namespace std;
using namespace util;

void StandardItem::setId(QString id) { id_ = ::move(id); }

void StandardItem::setText(QString text) { text_ = ::move(text); }

void StandardItem::setSubtext(QString subtext) { subtext_ = ::move(subtext); }

void StandardItem::setInputActionText(QString t) { input_action_text_ = ::move(t); }

void StandardItem::setIconUrls(QStringList icon_urls) { icon_urls_ = ::move(icon_urls); }

void StandardItem::setActions(vector<Action> actions) { actions_ = ::move(actions); }

QString StandardItem::id() const { return id_; }

QString StandardItem::text() const { return text_; }

QString StandardItem::subtext() const { return subtext_; }

QString StandardItem::inputActionText() const
{ return input_action_text_.isNull() ? text_ : input_action_text_; }

QStringList StandardItem::iconUrls() const { return icon_urls_; }

vector<Action> StandardItem::actions() const { return actions_; }
