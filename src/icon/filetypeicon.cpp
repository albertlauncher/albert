// SPDX-FileCopyrightText: 2022-2025 Manuel Schneider

#include "filetypeicon.h"
#include "systemutil.h"
#include <QFileIconProvider>
static QFileIconProvider qfip;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

FileTypeIcon::FileTypeIcon(const QString &path)
    : QIconIcon(qfip.icon(QFileInfo(path)))
    , path_(path)
{}

FileTypeIcon::FileTypeIcon(const filesystem::path &path)
    : FileTypeIcon(toQString(path))
{}

unique_ptr<Icon> FileTypeIcon::clone() const { return make_unique<FileTypeIcon>(*this); }

QString FileTypeIcon::toUrl() const { return u"%1:%2"_s.arg(scheme(), path_); }

QString FileTypeIcon::scheme() { return u"qfip"_s; }

unique_ptr<FileTypeIcon> FileTypeIcon::fromUrl(const QString &url)
{ return make_unique<FileTypeIcon>(url.mid(scheme().size() + 1 )); }
