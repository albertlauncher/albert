// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "qiconicon.h"
#include <filesystem>

namespace albert {

class ALBERT_EXPORT ImageIcon : public QIconIcon
{
public:
    ImageIcon(const QString &path);
    ImageIcon(const std::filesystem::path &path);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<ImageIcon> fromUrl(const QString &url);
    static QString fileScheme();
    static QString qrcScheme();
    static inline std::unique_ptr<Icon> make(auto&& path)
    { return std::make_unique<ImageIcon>(std::forward<decltype(path)>(path)); }

private:
    QString path_;
};

} // namespace albert
