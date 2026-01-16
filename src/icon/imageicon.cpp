// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "imageicon.h"
#include "systemutil.h"
#include <QFile>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

// TODO use imageloader

ImageIcon::ImageIcon(const QString &path)
    : QIconIcon(QFile::exists(path) ? QIcon(path) : QIcon())  // QIcon produces non null icons from non existing files
    , path_(path)
{}

ImageIcon::ImageIcon(const filesystem::path &path)
    : ImageIcon(toQString(path))
{}

unique_ptr<Icon> ImageIcon::clone() const { return make_unique<ImageIcon>(*this); }

QString ImageIcon::toUrl() const { return u"%1:%2"_s.arg(fileScheme(), path_); }

QString ImageIcon::fileScheme() { return u"file"_s; }

QString ImageIcon::qrcScheme() { return u"qrc"_s; }

unique_ptr<ImageIcon> ImageIcon::fromUrl(const QString &url)
{ return make_unique<ImageIcon>(url.mid(fileScheme().size() + 1)); }
