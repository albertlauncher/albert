// Copyright (c) 2026-2026 Manuel Schneider

#include "app.h"
#include "logging.h"
#include "pathmanager.h"
#include <QSettings>
using namespace albert;
using namespace std;

namespace {

static const char *CFG_ADDITIONAL_PATH_ENTRIES = "additional_path_entires";

static QStringList readPathVariable()
{ return qEnvironmentVariable("PATH").split(u':', Qt::SkipEmptyParts); }

static void setPathVariable(const QStringList &paths)
{
    auto var = paths.join(u':').toUtf8();
    qputenv("PATH", var);
    DEBG << "Effective PATH: " << var;
}

}

PathManager::PathManager(const QSettings &settings) :
    original_path_entries_(readPathVariable()),
    additional_path_entries_(settings.value(CFG_ADDITIONAL_PATH_ENTRIES).toStringList())
{
    setPathVariable(additional_path_entries_ + original_path_entries_);
}

const QStringList &PathManager::originalPathEntries() const { return original_path_entries_; }

const QStringList &PathManager::additionalPathEntries() const { return additional_path_entries_; }

void PathManager::setAdditionalPathEntries(const QStringList &entries)
{
    if (additional_path_entries_ == entries)
        return;

    additional_path_entries_ = entries;
    app().settings()->setValue(CFG_ADDITIONAL_PATH_ENTRIES, entries);
    setPathVariable(additional_path_entries_ + original_path_entries_);
}
