// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include <QIconEngine>
#include <memory>
namespace albert { class Icon; }

class QIconEngineAdapter : public QIconEngine
{
    std::unique_ptr<albert::Icon> icon_;
public:
    QIconEngineAdapter(std::unique_ptr<albert::Icon> icon);
    ~QIconEngineAdapter() override;

    QSize actualSize(const QSize &device_dependent_size, QIcon::Mode, QIcon::State) override;
    QPixmap pixmap(const QSize &device_dependent_size, QIcon::Mode, QIcon::State) override;
    QPixmap scaledPixmap(const QSize &device_independent_size, QIcon::Mode, QIcon::State, qreal scale) override;
    QIconEngine* clone() const override;
    QString iconName() override;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State) override;
    bool isNull() override;
};
