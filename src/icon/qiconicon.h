// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QIcon>

namespace albert {

class QIconIcon : public albert::Icon
{
public:
    QIconIcon(const QIcon &icon);

    QSize actualSize(const QSize&, double) override;
    QPixmap pixmap(const QSize&, double) override;
    void paint(QPainter*, const QRect&) override;
    bool isNull() override;

private:
    QIcon icon_;
};

} // namespace albert
