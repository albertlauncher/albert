// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/logging.h"
#include "qtpluginloader.h"
#include "qtpluginprovider.h"
#include <QCoreApplication>
#include <QDirIterator>
using namespace std;
using namespace albert;


QtPluginProvider::QtPluginProvider(QStringList paths)
{
#if defined __linux__ || defined __FreeBSD__
    paths << "../lib";
#elif defined __APPLE__
    paths << "../../../../lib";  // ./bin/albert.app/Contents/MacOS/
#endif

    QStringList install_paths;
#if defined __linux__ || defined __FreeBSD__
    if (qgetenv("container") == "flatpak")
        install_paths << "/app/lib/";
    install_paths << QDir::home().filePath(".local/lib/");
    install_paths << QDir::home().filePath(".local/lib64/");
    install_paths << "/usr/local/lib/";
    install_paths << "/usr/local/lib64/";
#if defined MULTIARCH_TUPLE
    install_paths << "/usr/lib/" MULTIARCH_TUPLE;
#endif
    install_paths << "/usr/lib/";
    install_paths << "/usr/lib64/";
#elif defined __APPLE__
    install_paths << QDir::home().filePath("Library/Application Support/albert/PlugIns");
    install_paths << QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../PlugIns");
#endif
    for (const QString& p : install_paths)
        paths << QDir(p).filePath("albert");

    QStringList unique_canonical_paths;
    for (const QString& p : paths)
        if (auto pfi = QFileInfo(p); pfi.isDir())  // implicit exists()
            unique_canonical_paths << pfi.canonicalFilePath();
    unique_canonical_paths.removeDuplicates();

    INFO << "Searching native plugins in" << unique_canonical_paths.join(", ");
    for (const auto &path : unique_canonical_paths)
    {
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
