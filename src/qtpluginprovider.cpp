// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/logging.h"
#include "qtpluginloader.h"
#include "qtpluginprovider.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>
using namespace std;
using namespace albert;

#if defined __linux__ || defined __FreeBSD__
static QStringList defaultPaths()
{
    QStringList default_paths;
    QStringList dirs = {
        QDir::home().filePath(".local/lib/"),
        QDir::home().filePath(".local/lib64/"),
        QFileInfo("/usr/local/lib/").canonicalFilePath(),
        QFileInfo("/usr/local/lib64/").canonicalFilePath(),
#if defined MULTIARCH_TUPLE
        QFileInfo("/usr/lib/" MULTIARCH_TUPLE).canonicalFilePath(),
#endif
        QFileInfo("/usr/lib/").canonicalFilePath(),
        QFileInfo("/usr/lib64/").canonicalFilePath(),
    };
    dirs.removeDuplicates();
    for ( const QString& dir : dirs ) {
        QFileInfo fileInfo = QFileInfo(QDir(dir).filePath("albert"));
        if ( fileInfo.isDir() )
            default_paths.push_back(fileInfo.canonicalFilePath());
    }
    return default_paths;
}
#elif defined __APPLE__
#include <QCoreApplication>
static QStringList defaultPaths()
{
    return {QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../PlugIns/albert")};
}
#endif

QtPluginProvider::QtPluginProvider(QStringList additional_paths):
    paths_(additional_paths << defaultPaths())
{
    /// Lookup is fast and frontend needed before the registry calls
    /// plugins(), therefore lookup actually happens in ctor
    for (const auto &path : paths_) {
        DEBG << "Searching native plugins in" << path;
        QDirIterator dirIterator(path, QDir::Files);
        while (dirIterator.hasNext()) {
            try {
                auto loader = make_unique<QtPluginLoader>(*this, QFileInfo(dirIterator.next()).absoluteFilePath());
                DEBG << "Found valid native plugin" << loader->path;
                plugins_.push_back(::move(loader));
            } catch (const runtime_error &e) {
                DEBG << e.what() << dirIterator.filePath();
            }
        }
    }
}

QString QtPluginProvider::id() const { return "pluginprovider"; }

QString QtPluginProvider::name() const { return "Native plugin provider"; }

QString QtPluginProvider::description() const { return "Loads native C++ albert plugins"; }

vector<PluginLoader*> QtPluginProvider::plugins()
{
    vector<PluginLoader*> r;
    for (const auto &loader : plugins_)
        r.emplace_back(loader.get());
    return r;
}

vector<QtPluginLoader*> QtPluginProvider::frontendPlugins()
{
    vector<QtPluginLoader*> frontend_plugins;
    for (auto &loader : plugins_)
        if (loader->metaData().load_type == LoadType::Frontend)
            frontend_plugins.emplace_back(loader.get());
    return frontend_plugins;
}
