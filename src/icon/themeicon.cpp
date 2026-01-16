// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "themeicon.h"
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include "iconlookup.h"
#endif
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;


ThemeIcon::ThemeIcon(const QString &name)
    : QIconIcon(
        // https://bugreports.qt.io/browse/QTBUG-135159
        // https://codereview.qt-project.org/c/qt/qtbase/+/634907
#if QT_VERSION > 0x060800
        QIcon::fromTheme(name)
#else
        QIcon(XDG::IconLookup::iconPath(name))
#endif
    )
    , name_(name)
{}

unique_ptr<Icon> ThemeIcon::clone() const { return make_unique<ThemeIcon>(*this); }

QString ThemeIcon::toUrl() const { return u"%1:%2"_s.arg(scheme(), name_); }

QString ThemeIcon::scheme() { return u"xdg"_s; }

unique_ptr<ThemeIcon> ThemeIcon::fromUrl(const QString &url)
{ return make_unique<ThemeIcon>(url.mid(scheme().size() + 1 )); }

