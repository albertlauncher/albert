// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/extensionregistry.h"
#include "albert/extensions/frontend.h"
#include "albert/logging.h"
#include "nativepluginprovider.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPluginLoader>
#include <QSettings>
#include <set>
using namespace std;
using namespace albert;

static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static const QRegularExpression regex_iid = QRegularExpression(R"R(org.albert.PluginInterface/(\d+).(\d+))R");
static const QRegularExpression regex_version = QRegularExpression(R"(^\d+\.\d+$)");
static const QRegularExpression regex_id = QRegularExpression("[a-z0-9_]");

static NativePluginMetaData *current_meta_data = nullptr;
static ExtensionRegistry *extension_registry = nullptr;

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
    d.cd("../lib");
    default_paths.push_back(d.canonicalPath()); // TODO deplopyment?
    d.cd("../Resources");
    default_paths.push_back(d.canonicalPath()); // TODO deplopyment?
#elif defined _WIN32
    qFatal("Not implemented");
#endif
    return default_paths;
}

NativePluginLoader::NativePluginLoader(NativePluginProvider *provider, ExtensionRegistry &registry, const QString &p)
    : PluginLoader(p), provider_(provider), registry_(registry)
{
    QPluginLoader loader(path);

    // Extract metadata

    metadata_.iid = loader.metaData()["IID"].toString();
    if (metadata_.iid.isEmpty())
        throw runtime_error("Not an albert plugin");

    auto rawMetadata = loader.metaData()["MetaData"].toObject();
    metadata_.id = rawMetadata["id"].toString();
    metadata_.version = rawMetadata["version"].toString();
    metadata_.name = rawMetadata["name"].toString();
    metadata_.description = rawMetadata["description"].toString();
    metadata_.long_description = rawMetadata["long_description"].toString();
    metadata_.license = rawMetadata["license"].toString();
    metadata_.url = rawMetadata["url"].toString();
    metadata_.maintainers = rawMetadata["maintainers"].toVariant().toStringList();
    metadata_.runtime_dependencies = rawMetadata["runtime_dependencies"].toVariant().toStringList();
    metadata_.binary_dependencies = rawMetadata["binary_dependencies"].toVariant().toStringList();
    metadata_.third_party_credits = rawMetadata["credits"].toVariant().toStringList();
    metadata_.frontend = rawMetadata["frontend"].toBool();
    metadata_.user = !metadata_.frontend;

    // Validate metadata

    QStringList errors;

    if (auto iid_match = regex_iid.match(metadata_.iid); !iid_match.hasMatch())
        errors << QString("Invalid IID pattern: '%1'. Expected '%2'.")
                      .arg(iid_match.captured(), iid_match.regularExpression().pattern());
    else if (auto plugin_iid_major = iid_match.captured(1).toUInt(); plugin_iid_major != ALBERT_VERSION_MAJOR)
            errors << QString("Incompatible major version: %1. Expected: %2.")
                          .arg(plugin_iid_major).arg(ALBERT_VERSION_MAJOR);
    else if (auto plugin_iid_minor = iid_match.captured(2).toUInt(); plugin_iid_minor > ALBERT_VERSION_MINOR)
            errors << QString("Incompatible minor version: %1. Supported up to: %2.")
                          .arg(plugin_iid_minor).arg(ALBERT_VERSION_MINOR);

    if (!regex_version.match(metadata_.version).hasMatch())
        errors << "Invalid version scheme. Use '<version>.<patch>'.";

    if (!regex_id.match(metadata_.id).hasMatch())
        errors << "Invalid plugin id. Use [a-z0-9_].";

    if (metadata_.name.isEmpty())
        errors << "'name' must not be empty.";

    if (metadata_.description.isEmpty())
        errors << "'description' must not be empty.";

    // Finally set state based on errors

    if (errors.isEmpty())
        state_ = PluginState::Unloaded;
    else{
        WARN << QString("Plugin invalid: %1. (%2)").arg(errors.join(", "), path);
        state_info_ = errors.join(", ");
    }
}

NativePluginLoader::~NativePluginLoader()
{
    if (state_ == PluginState::Loaded)
        NativePluginLoader::unload();
}

NativePluginInstance *NativePluginLoader::instance() const { return instance_; }

NativePluginProvider *NativePluginLoader::provider() const { return provider_; }

const NativePluginMetaData &NativePluginLoader::metaData() const { return metadata_; }

void NativePluginLoader::load()
{
    if (state_ == PluginState::Invalid)
        qFatal("Loaded an invalid plugin.");
    else if (state_ == PluginState::Loaded)
        return;

    QPluginLoader loader(path);
    // Some python libs do not link against python. Export the python symbols to the main app.
    loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);// | QLibrary::PreventUnloadHint);
    try {
        // inject using static vars for default constructability
        current_meta_data = &metadata_;
        extension_registry = &registry_;

        if (auto *instance = loader.instance()){
            if ((instance_ = dynamic_cast<NativePluginInstance*>(instance))){
                state_ = PluginState::Loaded;
                state_info_.clear();
                if (auto *e = dynamic_cast<Extension*>(instance))  // Auto registration
                    registry_.add(e);
                return;
            } else
                state_info_ = "Plugin is not of type Plugin";
        } else
            state_info_ = loader.errorString();
    } catch (const exception& e) {
        state_info_ = e.what();
    } catch (...) {
        state_info_ = "Unknown exception.";
    }
    state_ = PluginState::Unloaded;
    loader.unload();
}

void NativePluginLoader::unload()
{
    if (state_ == PluginState::Invalid)
        qFatal("Unloaded an invalid plugin.");
    else if (state_ == PluginState::Unloaded)
        return;

    if (auto *e = dynamic_cast<Extension*>(instance_))  // Auto deregistration
        registry_.remove(e);
    QPluginLoader loader(path);
    delete instance_;  // TODO this saves from segfaults but actually this is QPluginLoaders job
    instance_ = nullptr;
    loader.unload();
    state_info_.clear();
    state_ = PluginState::Unloaded;
}


// ///////////////////////////////////////////////////////////////////////////////////////////// //


class NativePluginInstance::Private
{
public:
    NativePluginMetaData &metaData = *current_meta_data;
    ExtensionRegistry &registry = *extension_registry;
};

NativePluginInstance::NativePluginInstance(): d(std::make_unique<Private>()) {}

NativePluginInstance::~NativePluginInstance() = default;

ExtensionRegistry &NativePluginInstance::registry()
{
    return d->registry;
}

const NativePluginMetaData &NativePluginInstance::metaData() const { return d->metaData; }



// ///////////////////////////////////////////////////////////////////////////////////////////// //


QString ExtensionPlugin::id() const { return metaData().id; }

QString ExtensionPlugin::name() const { return metaData().name; }

QString ExtensionPlugin::description() const { return metaData().description; }


// ///////////////////////////////////////////////////////////////////////////////////////////// //


NativePluginProvider::NativePluginProvider(ExtensionRegistry &registry, const QStringList &additional_paths):
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
                auto loader = make_unique<NativePluginLoader>(this, registry, dirIterator.next());
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

NativePluginProvider::~NativePluginProvider()
{
    for (auto &loader : plugins_)
        if (loader->state() == PluginState::Loaded)
            loader->unload();
}

void NativePluginProvider::loadFrontend()
{
    DEBG << "Loading frontend plugin…";

    // Helper function loading frontend extensions
    auto load_frontend = [](NativePluginLoader *loader) -> Frontend* {

        if (loader->load(); loader->state() == PluginState::Loaded){
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
    auto cfg_frontend = QSettings(qApp->applicationName()).value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    if (auto it = find_if(frontend_plugins_.begin(), frontend_plugins_.end(),
                          [&](const NativePluginLoader *loader){ return cfg_frontend == loader->metaData().id; });
            it == frontend_plugins_.end())
        WARN << "Configured frontend does not exist: " << cfg_frontend;
    else if (frontend_ = load_frontend(*it); frontend_)
        return;
    else
        WARN << "Loading configured frontend failed. Try any other.";

    for (auto &loader : frontend_plugins_)
        if (frontend_ = load_frontend(loader); frontend_) {
            WARN << QString("Using %1 instead.").arg(loader->metaData().id);
            QSettings(qApp->applicationName()).setValue(CFG_FRONTEND_ID, loader->metaData().id);
            return;
        }
    qFatal("Could not load any frontend.");
}

Frontend *NativePluginProvider::frontend() { return frontend_; }

const vector<NativePluginLoader*> &NativePluginProvider::frontendPlugins() { return frontend_plugins_; }

void NativePluginProvider::setFrontend(uint index)
{
    auto id = frontend_plugins_[index]->metaData().id;
    QSettings(qApp->applicationName()).setValue(CFG_FRONTEND_ID, id);
    if (id != frontend_->id()){
        QMessageBox msgBox(QMessageBox::Question, "Restart?",
                           "Changing the frontend needs a restart. Do you want to restart Albert?",
                           QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes)
            restart();
    }
}


// Interfaces

QString NativePluginProvider::id() const { return "pluginprovider"; }

QString NativePluginProvider::name() const { return "Native plugin provider"; }

QString NativePluginProvider::description() const { return "Loads native C++ albert plugins"; }

vector<PluginLoader*> NativePluginProvider::plugins()
{
    vector<PluginLoader*> r;
    for (const auto &loader : plugins_)
        r.emplace_back(loader.get());
    return r;
}
