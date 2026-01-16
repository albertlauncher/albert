// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "qiconicon.h"
#include <QString>

namespace albert {

class ALBERT_EXPORT ThemeIcon : public QIconIcon
{
public:
    ThemeIcon(const QString &name);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<ThemeIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&& name)
    { return std::make_unique<ThemeIcon>(std::forward<decltype(name)>(name)); }

private:
    QString name_;
};

} // namespace albert
