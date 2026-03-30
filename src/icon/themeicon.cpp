// SPDX-FileCopyrightText: 2022-2026 Manuel Schneider

#include "themeicon.h"
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

ThemeIcon::ThemeIcon(const QString &name)
    : QIconIcon(QIcon::fromTheme(name))
    , name_(name)
{}

unique_ptr<Icon> ThemeIcon::clone() const { return make_unique<ThemeIcon>(*this); }

QString ThemeIcon::toUrl() const { return u"%1:%2"_s.arg(scheme(), name_); }

QString ThemeIcon::scheme() { return u"xdg"_s; }

unique_ptr<ThemeIcon> ThemeIcon::fromUrl(const QString &url)
{ return make_unique<ThemeIcon>(url.mid(scheme().size() + 1 )); }
