// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QIcon>
#include <QBrush>
#include <memory>

namespace albert {

class ALBERT_EXPORT IconifiedIcon : public albert::Icon
{
public:
    IconifiedIcon(std::unique_ptr<Icon> icon,
                  const QBrush &background_brush = default_background_brush,
                  double radius = default_border_radius,
                  int border_width = default_border_width,
                  const QBrush &border_brush = default_border_brush);


    QSize actualSize(const QSize&, double) override;
    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<IconifiedIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&&... args)
    { return std::make_unique<IconifiedIcon>(std::forward<decltype(args)>(args)...); }

    static QBrush default_background_brush;
    static double default_border_radius;
    static int    default_border_width;
    static QBrush default_border_brush;

private:
    std::unique_ptr<Icon> src_;
    const QBrush color_;
    const double border_radius_;
    const int    border_width_;
    const QBrush border_brush_;
};

} // namespace albert
