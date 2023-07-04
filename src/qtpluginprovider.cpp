// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/frontend/frontend.h"
#include "albert/logging.h"
#include "qtpluginloader.h"
#include "qtpluginprovider.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
using namespace std;
using namespace albert;

static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";

static QStringList defaultPaths()
{
    QStringList default_paths;
#if defined __linux__ || defined __FreeBSD__
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
#elif defined __APPLE__
    QDir d(QCoreApplication::applicationDirPath());
    d.cd("../PlugIns");
    default_paths.push_back(d.canonicalPath());
#elif defined _WIN32
    qFatal("Not implemented");
#endif
    return default_paths;
}

QtPluginProvider::QtPluginProvider(const QStringList &additional_paths):
    frontend_(nullptr)
{
    QStringList paths;
    if (!additional_paths.isEmpty())
        paths << additional_paths;
    paths << defaultPaths();

    for (const auto &path : paths) {
        DEBG << "Searching native plugins in" << path;
        QDirIterator dirIterator(path, QDir::Files);
        while (dirIterator.hasNext()) {
            try {
                auto loader = make_unique<QtPluginLoader>(this, dirIterator.next());
                if (loader->metaData().frontend)
                    frontend_plugins_.emplace_back(loader.get());
                DEBG << "Found valid native plugin" << loader->path;
                plugins_.push_back(::move(loader));
            } catch (const runtime_error &e) {
                DEBG << e.what() << dirIterator.filePath();
            }
        }
    }

    if (frontend_plugins_.empty())
        qFatal("No frontends found.");
}

QtPluginProvider::~QtPluginProvider()
{
    for (auto &loader : plugins_)
        if (loader->state() == PluginLoader::PluginState::Loaded)
            loader->unload();
}

void QtPluginProvider::loadFrontend()
{
    DEBG << "Loading frontend plugin…";

    // Helper function loading frontend extensions
    auto load_frontend = [](QtPluginLoader *loader) -> Frontend* {

        if (loader->load(); loader->state() == PluginLoader::PluginState::Loaded){
            if (auto *f = dynamic_cast<Frontend*>(loader->instance()))
                return f;
            else{
                DEBG << "Failed casting Plugin instance to Frontend*";
                loader->unload();
            }
        } else
            DEBG << loader->stateInfo();
        return nullptr;  // Loading failed
    };

    // Try loading the configured frontend
    auto cfg_frontend = albert::settings()->value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    if (auto it = find_if(frontend_plugins_.begin(), frontend_plugins_.end(),
                          [&](const QtPluginLoader *loader){ return cfg_frontend == loader->metaData().id; });
            it == frontend_plugins_.end())
        WARN << "Configured frontend does not exist: " << cfg_frontend;
    else if (frontend_ = load_frontend(*it); frontend_)
        return;
    else
        WARN << "Loading configured frontend failed. Try any other.";

    for (auto &loader : frontend_plugins_)
        if (frontend_ = load_frontend(loader); frontend_) {
            WARN << QString("Using %1 instead.").arg(loader->metaData().id);
            albert::settings()->setValue(CFG_FRONTEND_ID, loader->metaData().id);
            return;
        }
    qFatal("Could not load any frontend.");
}

Frontend *QtPluginProvider::frontend() { return frontend_; }

const vector<QtPluginLoader*> &QtPluginProvider::frontendPlugins() { return frontend_plugins_; }

void QtPluginProvider::setFrontend(uint index)
{
    auto id = frontend_plugins_[index]->metaData().id;
    albert::settings()->setValue(CFG_FRONTEND_ID, id);
    if (id != frontend_->id()){
        QMessageBox msgBox(QMessageBox::Question, "Restart?",
                           "Changing the frontend needs a restart. Do you want to restart Albert?",
                           QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes)
            restart();
    }
}


// Interfaces

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
