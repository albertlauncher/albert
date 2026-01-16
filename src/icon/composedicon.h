// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QBrush>
#include <memory>

namespace albert {

class ALBERT_EXPORT ComposedIcon : public albert::Icon
{
public:
    ComposedIcon(std::unique_ptr<Icon> src1,
                 std::unique_ptr<Icon> src2,
                 double size1 = default_size,
                 double size2 = default_size,
                 double x1 = default_pos1,
                 double y1 = default_pos1,
                 double x2 = default_pos2,
                 double y2 = default_pos2);

    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<ComposedIcon> fromUrl(const QString &url);
    static QString scheme();
    static inline std::unique_ptr<Icon> make(auto&&... args)
    { return std::make_unique<ComposedIcon>(std::forward<decltype(args)>(args)...); }

    static double default_size;
    static double default_pos1;
    static double default_pos2;

private:
    std::shared_ptr<Icon> src1_;
    std::shared_ptr<Icon> src2_;
    double size1_, size2_, x1_, y1_, x2_, y2_;
};

} // namespace albert
