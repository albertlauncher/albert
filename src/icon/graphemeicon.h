// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QBrush>
#include <QString>

namespace albert {

class ALBERT_EXPORT GraphemeIcon : public albert::Icon
{
public:
    GraphemeIcon(const QString &grapheme,
                 double scalar = 1.0,
                 const QBrush &color = defaultBrush());

    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<GraphemeIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&&... args)
    { return std::make_unique<GraphemeIcon>(std::forward<decltype(args)>(args)...); }

    static QBrush defaultBrush();

private:
    const QString grapheme_;
    const double scalar_;
    const QBrush color_;
};

} // namespace albert
