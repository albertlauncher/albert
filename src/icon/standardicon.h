// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "qiconicon.h"

namespace albert {

class ALBERT_EXPORT StandardIcon : public QIconIcon
{
public:

    StandardIcon(StandardIconType type);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<StandardIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline auto make(StandardIconType type)
    { return std::make_unique<StandardIcon>(type); }

private:
    StandardIconType type_;
};

} // namespace albert
