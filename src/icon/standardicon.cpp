// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "standardicon.h"
#include "logging.h"
#include <QApplication>
#include <QMetaEnum>
#include <QStyle>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

// https://doc.qt.io/qt-6/qstyle.html#StandardPixmap-enum

StandardIcon::StandardIcon(StandardIconType type)
    : QIconIcon(qApp->style()->standardIcon((QStyle::StandardPixmap)type))
    , type_(type)
{}

unique_ptr<Icon> StandardIcon::clone() const { return make_unique<StandardIcon>(*this); }

QString StandardIcon::toUrl() const
{
    return u"%1:%2"_s.arg(scheme(), QMetaEnum::fromType<QStyle::StandardPixmap>().key(type_));
}

QString StandardIcon::scheme() { return u"qsp"_s; }

unique_ptr<StandardIcon> StandardIcon::fromUrl(const QString &url)
{
    const auto enum_key = url.mid(scheme().size() + 1);

    const auto meta_enum = QMetaEnum::fromType<QStyle::StandardPixmap>();
    for (int i = 0; i < meta_enum.keyCount(); ++i)
        if (enum_key == meta_enum.key(i))
            return make_unique<StandardIcon>(static_cast<StandardIconType>(meta_enum.value(i)));

    WARN << "No such QStyle::StandardPixmap key found:" << enum_key;
    return nullptr;
}

