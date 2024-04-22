// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/logging.h"
#include "qtpluginloader.h"
#include "qtpluginprovider.h"
#include <QCoreApplication>
#include <QDirIterator>
using namespace std;
using namespace albert;


QtPluginProvider::QtPluginProvider(QStringList additional_paths)
{
#if defined __linux__ || defined __FreeBSD__

    QStringList default_paths = {
        QDir::home().filePath(".local/lib/"),
        QDir::home().filePath(".local/lib64/"),
        "/usr/local/lib/",
        "/usr/local/lib64/",
#if defined MULTIARCH_TUPLE
        "/usr/lib/" MULTIARCH_TUPLE,
#endif
        "/usr/lib/",
        "/usr/lib64/"
    };

#elif defined __APPLE__

    QStringList default_paths = {
        QDir::home().filePath("Library/Application Support/albert/PlugIns"),
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../PlugIns")
    };

#endif

    for (const QString& default_path : default_paths)
        if (auto fi = QFileInfo(QDir(default_path).filePath("albert")); fi.isDir())  // implicit exists()
            additional_paths << fi.canonicalFilePath();

    additional_paths.removeDuplicates();

    for (const auto &path : additional_paths)
    {
        DEBG << "Searching native plugins in" << path;

        QDirIterator dirIterator(path, QDir::Files);
        while (dirIterator.hasNext()) {
            try {
                auto pl = make_unique<QtPluginLoader>(QFileInfo(dirIterator.next()).absoluteFilePath());
                DEBG << "Found valid native plugin" << pl->path();
                plugin_loaders_.emplace_back(::move(pl));
            } catch (const runtime_error &e) {
                DEBG << dirIterator.filePath() << e.what();
            }
        }
    }
}

QtPluginProvider::~QtPluginProvider() = default;

QString QtPluginProvider::id() const { return QStringLiteral("qtpluginprovider"); }

QString QtPluginProvider::name() const { return QStringLiteral("C++/Qt"); }

QString QtPluginProvider::description() const
{
    static const auto tr = QCoreApplication::translate("QtPluginProvider", "Loads native C++ plugins");
    return tr;
}

vector<PluginLoader*> QtPluginProvider::plugins()
{
    vector<PluginLoader*> plugins;
    for (const auto &pl : plugin_loaders_)
        if (pl->metaData().load_type == PluginMetaData::LoadType::User)
            plugins.emplace_back(pl.get());
    return plugins;
}

vector<PluginLoader*> QtPluginProvider::frontendPlugins()
{
    vector<PluginLoader*> frontend_plugins;
    for (const auto &pl : plugin_loaders_)
        if (pl->metaData().load_type == PluginMetaData::LoadType::Frontend)
            frontend_plugins.emplace_back(pl.get());
    return frontend_plugins;
}
