// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <QObject>
#include <QRunnable>
#include <QMutex>

namespace ChromeBookmarks {
class Extension;

class Indexer final : public QObject,  public QRunnable
{
    Q_OBJECT
public:
    Indexer(Extension *ext)
        : _extension(ext), _abort(false) {}
    void run() override;
    void abort(){_abort=true;}

private:
    Extension *_extension;
    bool _abort;

signals:
    void statusInfo(const QString&);
};
}
