// Copyright (c) 2022-2025 Manuel Schneider

#include "pathmanager.h"
#include "logging.h"
#include "app.h"
#include <QStringList>
#include <QSettings>
#include <QtGlobal>
using namespace albert;
using namespace std;

static const char *CFG_ADDITIONAL_PATH_ENTRIES = "additional_path_entires";

class PathManager::Private
{
public:
    const QStringList original_path_entries = qEnvironmentVariable("PATH").split(u':', Qt::SkipEmptyParts);
    QStringList additional_path_entries;
};

PathManager::PathManager(const QSettings &settings) : d(make_unique<Private>())
{
    d->additional_path_entries = settings.value(CFG_ADDITIONAL_PATH_ENTRIES).toStringList();
    auto effective_path_entries = QStringList() << d->additional_path_entries
                                                << d->original_path_entries;
    auto new_path = effective_path_entries.join(u':').toUtf8();
    qputenv("PATH", new_path);
    DEBG << "Effective PATH: " << new_path;
}

PathManager::~PathManager() {}

const QStringList &PathManager::originalPathEntries() const { return d->original_path_entries; }

const QStringList &PathManager::additionalPathEntries() const { return d->additional_path_entries; }

void PathManager::setAdditionalPathEntries(const QStringList &entries)
{
    if (entries != d->additional_path_entries)
    {
        d->additional_path_entries = entries;
        App::instance().settings()->setValue(CFG_ADDITIONAL_PATH_ENTRIES, entries);
    }
}
