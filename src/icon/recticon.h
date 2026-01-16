// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QBrush>

namespace albert {

class ALBERT_EXPORT RectIcon : public albert::Icon
{
public:
    RectIcon(const QBrush &color = defaultColor(),
             double radius = defaultRadius(),
             int border_width = defaultBorderWidth(),
             const QBrush &border_color = defaultBorderColor());

    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<RectIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&&... args)
    { return std::make_unique<RectIcon>(std::forward<decltype(args)>(args)...); }

    static QBrush defaultColor();
    static double defaultRadius();
    static int defaultBorderWidth();
    static QBrush defaultBorderColor();

private:
    const QBrush color_;
    const double radius_;
    const int border_width_;
    const QBrush border_color_;
};

} // namespace albert
