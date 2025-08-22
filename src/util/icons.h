// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#pragma once
#include "icon.h"
#include <QBrush>
#include <QIcon>
#include <filesystem>
class QMetaEnum;


struct ALBERT_EXPORT QIconIcon : public albert::Icon
{
    QIconIcon(const QIcon &icon);

    QSize actualSize(const QSize&, double) override;
    QPixmap pixmap(const QSize&, double) override;
    void paint(QPainter*, const QRect&) override;
    bool isNull() override;

    QIcon icon_;
};


struct ALBERT_EXPORT ImageIcon : public QIconIcon
{
    ImageIcon(const QString &path);
    ImageIcon(const std::filesystem::path &path);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<ImageIcon> fromUrl(const QString &url);

    QString path_;
    static const QString scheme;
    static const QString qrc_scheme;
};


struct ALBERT_EXPORT FileTypeIcon : public QIconIcon
{
    FileTypeIcon(const QString &path);
    FileTypeIcon(const std::filesystem::path &path);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<FileTypeIcon> fromUrl(const QString &url);

    QString path_;
    static const QString scheme;
};


struct ALBERT_EXPORT StandardIcon : public QIconIcon
{
    StandardIcon(int standard_icon_enum_value);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<StandardIcon> fromUrl(const QString &url);

    int standard_icon_enum_value_;
    static const QString scheme;
    static const QMetaEnum meta_enum;
};


struct ALBERT_EXPORT ThemeIcon : public QIconIcon
{
    ThemeIcon(const QString &name);

    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<ThemeIcon> fromUrl(const QString &url);

    QString name_;
    static const QString scheme;
};


struct ALBERT_EXPORT GraphemeIcon : public albert::Icon
{
    GraphemeIcon(const QString &grapheme,
                 double scalar = default_scalar,
                 const QBrush &color = default_color);


    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<GraphemeIcon> fromUrl(const QString &url);

    const QString grapheme_;
    const double scalar_;
    const QBrush color_;

    static const QString scheme;
    static const double default_scalar;
    static const QBrush default_color;
};


struct ALBERT_EXPORT RectIcon : public albert::Icon
{
    RectIcon(const QBrush &color = default_color,
             double radius = default_radius,
             int border_width = default_border_width,
             const QBrush &border_color = default_border_color);

    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<RectIcon> fromUrl(const QString &url);

    const QBrush color_;
    const double radius_;
    const int border_width_;
    const QBrush border_color_;

    static const QString scheme;
    static const QBrush  default_color;
    static const double  default_radius;
    static const int     default_border_width;
    static const QBrush  default_border_color;
};


struct ALBERT_EXPORT ComposedIcon : public albert::Icon
{
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

    std::shared_ptr<Icon> src1_;
    std::shared_ptr<Icon> src2_;
    double size1_, size2_, x1_, y1_, x2_, y2_;

    static const QString scheme;
    static const double default_size;
    static const double default_pos1;
    static const double default_pos2;
};


struct ALBERT_EXPORT IconifiedIcon : public albert::Icon
{
    IconifiedIcon(std::unique_ptr<Icon> src,
                  const QBrush &color = default_color,
                  double border_radius = default_border_radius,
                  int border_width = default_border_width,
                  const QBrush &border_color = default_border_color);

    QSize actualSize(const QSize&, double) override;
    void paint(QPainter*, const QRect&) override;
    bool isNull() override;
    std::unique_ptr<Icon> clone() const override;
    QString toUrl() const override;

    static std::unique_ptr<IconifiedIcon> fromUrl(const QString &url);

    std::unique_ptr<Icon> src_;
    const QBrush background_color_;
    const double border_radius_;
    const int    border_width_;
    const QBrush border_color_;

    static const QString scheme;
    static const QBrush  default_color;
    static const double  default_border_radius;
    static const int     default_border_width;
    static const QBrush  default_border_color;

};
