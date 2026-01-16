// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "qiconicon.h"
#include <filesystem>

namespace albert {

class ALBERT_EXPORT FileTypeIcon : public QIconIcon
{
public:
    FileTypeIcon(const QString &path);
    FileTypeIcon(const std::filesystem::path &path);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<FileTypeIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&& path)
    { return std::make_unique<FileTypeIcon>(std::forward<decltype(path)>(path)); }

private:
    QString path_;
};

} // namespace albert
