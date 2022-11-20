// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/extensions/frontend.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "albert/plugin.h"
#include "pluginprovider.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QPluginLoader>
#include <chrono>
using namespace std;
using namespace albert;
using chrono::duration_cast;
using chrono::milliseconds;
using chrono::system_clock;
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";

static const char *IID_PATTERN = R"R(org.albert.PluginInterface/(\d+).(\d+))R";
PluginSpec *current_spec_in_construction = nullptr;
ExtensionRegistry *extension_registry = nullptr;
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
        QFileInfo fileInfo = QFileInfo(QDir(dir).filePath("albert/plugins"));
        if ( fileInfo.isDir() )
            default_paths.push_back(fileInfo.canonicalFilePath());
    }
#elif defined __APPLE__
    default_paths.push_back(QDir("../lib").canonicalPath()); // TODO deplopyment?
#elif defined _WIN32
    qFatal("Not implemented");
#endif
    return default_paths;
}

NativePluginProvider::NativePluginProvider(albert::ExtensionRegistry &registry, const QStringList &additional_paths):
    registry_(registry), frontend_(nullptr)
{
    QStringList paths;
    paths << additional_paths;
    paths << defaultPaths();

    for (const auto &path : paths) {
        QDirIterator dirIterator(path, QDir::Files);
        while (dirIterator.hasNext()) {
            try {
                auto spec = parsePluginMetadata(dirIterator.next());
                if (const auto &[it, success] = plugins_.emplace(spec.id, spec); success){
                    if (spec.type == PluginSpec::Type::Frontend)
                        frontend_plugins_.emplace_back(&it->second);
                } else
                    WARN << "Plugin id already exists. Skip" << spec.path;
            } catch (const runtime_error &e) {
                WARN << e.what() << dirIterator.filePath();
            }
        }
    }

    if (frontend_plugins_.empty())
        qFatal("No frontends found.");

    extension_registry = &registry;
}

NativePluginProvider::~NativePluginProvider()
{
    for (auto &[id, spec] : plugins_)
        if (spec.state == PluginSpec::State::Loaded)
            unload(spec);
}

void NativePluginProvider::loadFrontend()
{
    DEBG << "Loading frontend plugin…";
    // Helper function loading frontend extensions
    auto load_frontend = [this](PluginSpec *spec) -> Frontend* {
        if (auto *p = load(*spec); p){
            if (auto *f = dynamic_cast<Frontend*>(p); f)
                return f;
            else
                unload(*spec);
        }
        return nullptr;  // Loading failed
    };

    // Try loading the configured frontend
    auto cfg_frontend = QSettings().value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    if (auto it = std::find_if(frontend_plugins_.begin(), frontend_plugins_.end(),
                               [&](const PluginSpec *spec){ return cfg_frontend == spec->id; });
            it == frontend_plugins_.end())
        WARN << "Configured frontend does not exist: " << cfg_frontend;
    else if (frontend_ = load_frontend(*it); frontend_)
        return;
    else
        WARN << "Loading configured frontend failed. Try any other.";

    for (auto &spec : frontend_plugins_)
        if (frontend_ = load_frontend(spec); frontend_) {
            WARN << QString("Using %1 instead.").arg(spec->id);
            return;
        }
    qFatal("Could not load any frontend.");
}

albert::Frontend *NativePluginProvider::frontend()
{
    return frontend_;
}

const std::vector<albert::PluginSpec*> &NativePluginProvider::frontendPlugins()
{
    return frontend_plugins_;
}

void NativePluginProvider::setFrontend(uint index)
{
    QSettings().setValue(CFG_FRONTEND_ID, frontend_plugins_[index]->id);
    if (frontend_plugins_[index]->id != frontend_->id()){
        QMessageBox msgBox(QMessageBox::Question, "Restart?",
                           "Changing the frontend needs a restart. Do you want to restart Albert?",
                           QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes)
            albert::restart();
    }
}


PluginSpec NativePluginProvider::parsePluginMetadata(const QString& path)
{
    QPluginLoader loader(path);
    QStringList errors;
    PluginSpec spec;
    spec.provider = static_cast<PluginProvider *>(this);
    spec.path = path;

    spec.iid = loader.metaData()["IID"].toString();
    if (spec.iid.isEmpty())
        throw runtime_error("Not a QPlugin");
    auto iid_match = QRegularExpression(IID_PATTERN).match(spec.iid);
    if (!iid_match.hasMatch())
        throw runtime_error(QString("Invalid IID pattern: '%1'. Expected '%2'.")
                                    .arg(iid_match.captured(), iid_match.regularExpression().pattern()).toStdString());
    else {
        if (auto plugin_iid_major = iid_match.captured(1).toUInt(); plugin_iid_major != ALBERT_VERSION_MAJOR)
            throw runtime_error(QString("Incompatible major version: %1. Expected: %2.")
                                        .arg(plugin_iid_major).arg(ALBERT_VERSION_MAJOR).toStdString());
        if (auto plugin_iid_minor = iid_match.captured(2).toUInt(); plugin_iid_minor > ALBERT_VERSION_MINOR)
            throw runtime_error(QString("Incompatible minor version: %1. Supported up to: %2.")
                                        .arg(plugin_iid_minor).arg(ALBERT_VERSION_MINOR).toStdString());
    }

    auto rawMetadata = loader.metaData()["MetaData"].toObject();
    spec.id = rawMetadata["id"].toString();
    spec.version = rawMetadata["version"].toString();
    spec.name = rawMetadata["name"].toString();
    spec.description = rawMetadata["description"].toString();
    spec.url = rawMetadata["url"].toString();
    spec.license = rawMetadata["license"].toString();
    spec.authors = rawMetadata["authors"].toVariant().toStringList();
    spec.maintainers = rawMetadata["maintainers"].toVariant().toStringList();
    spec.plugin_dependencies = rawMetadata["plugin_dependencies"].toVariant().toStringList();
    spec.runtime_dependencies = rawMetadata["runtime_dependencies"].toVariant().toStringList();
    spec.binary_dependencies = rawMetadata["binary_dependencies"].toVariant().toStringList();
    spec.third_party = rawMetadata["third_party"].toVariant().toStringList();
    if (auto string_type = rawMetadata["type"].toString(); string_type == "none")
        spec.type = PluginSpec::Type::None;
    else if (string_type == "frontend")
        spec.type = PluginSpec::Type::Frontend;
    else
        spec.type = PluginSpec::Type::User;

    if (!QRegularExpression(R"(^\d+\.\d+$)").match(spec.version).hasMatch())
        WARN << "Invalid version scheme. Use '<version>.<patch>'.";

    if (!QRegularExpression("[a-z0-9_]").match(spec.id).hasMatch())
        WARN << "Invalid plugin id. Use [a-z0-9_].";

    if (spec.name.isEmpty())
        WARN << "'name' should not be empty.";

    if (spec.description.isEmpty())
        WARN << "'description' should not be empty.";

    if (spec.url.isEmpty())
        WARN << "'url' should not be empty.";

    if (spec.license.isEmpty())
        WARN << "'license' should not be empty.";

    if (spec.authors.isEmpty())
        WARN << "'authors' should not be empty.";

    // Finally set state based on errors

    if (errors.isEmpty())
        spec.state = PluginSpec::State::Unloaded;
    else{
        WARN << errors.join(" ") << path;
        spec.state = PluginSpec::State::Error;
        spec.reason = errors.join(" ");
    }

    return spec;
}

// Interfaces

QString NativePluginProvider::id() const
{
    return "pluginprovider";
}

QString NativePluginProvider::name() const
{
    return "Native plugin provider";
}

QString NativePluginProvider::description() const
{
    return "Loads native C++ albert plugins";
}

QIcon NativePluginProvider::icon() const
{
    return QIcon(":cpp");
}

const map<QString,PluginSpec> &NativePluginProvider::plugins() const
{
    return plugins_;
}

bool NativePluginProvider::isEnabled(const QString &id)
{
    return QSettings().value(QString("%1/enabled").arg(id), false).toBool();
}

bool NativePluginProvider::setEnabled(const QString &id, bool enable)
{
    try{
        auto &spec = plugins_.at(id);
        QSettings().setValue(QString("%1/enabled").arg(id), enable);
        return enable ? (bool)load(spec) : unload(spec);
    } catch (const out_of_range &) {
        WARN << "Enabled nonexistant id:" << id;
        return false;
    }
}

Plugin *NativePluginProvider::load(PluginSpec &spec)
{
    if (spec.state == PluginSpec::State::Loaded)
        qFatal("Logic error: Loading a loaded plugin.");

    INFO << QString("Loading plugin '%1'").arg(spec.id);
    QPluginLoader loader(spec.path);
    // Some python libs do not link against python. Export the python symbols to the main app.
    loader.setLoadHints(QLibrary::ExportExternalSymbolsHint | QLibrary::PreventUnloadHint);

    QString error;
    auto start = system_clock::now();
    try {
        current_spec_in_construction = &spec;
        if (auto *instance = loader.instance()){
            if (auto *plugin = dynamic_cast<Plugin*>(instance)){
                spec.state = PluginSpec::State::Loaded;
                DEBG << QString("Plugin '%1' loaded (%2ms)").arg(spec.id)
                            .arg(duration_cast<milliseconds>(system_clock::now() - start).count());
                if (auto *e = dynamic_cast<Extension*>(instance))
                    registry_.add(e);  // Auto registration
                return plugin;
            } else
                error = "Plugin is not of type albert::Plugin";
        } else
            error = loader.errorString();
    } catch (const exception& e) {
        error = e.what();
    } catch (const string& s) {
        error = QString::fromStdString(s);
    } catch (const QString& s) {
        error = s;
    } catch (const char *s) {
        error = s;
    } catch (...) {
        error = "Unknown exception.";
    }

    DEBG << "Loading plugin failed:" << error;
    loader.unload();
    spec.state = PluginSpec::State::Error;
    spec.reason = error;
    return nullptr;
}

bool NativePluginProvider::unload(PluginSpec &spec)
{
    if (spec.state != PluginSpec::State::Loaded)
        qFatal("Logic error: Unloading an unloaded plugin.");

    DEBG << "Unloading plugin" << spec.id;
    QPluginLoader loader(spec.path);
    if (auto *e = dynamic_cast<Extension*>(loader.instance()))
        registry_.remove(e);  // Auto deregistration
    auto start = system_clock::now();
    bool success = loader.unload();
    if (success){
        spec.state = PluginSpec::State::Unloaded;
        DEBG << QString("%1 unloaded in %2 milliseconds").arg(spec.id)
                .arg(duration_cast<milliseconds>(system_clock::now()-start).count());
    } else {
        spec.state = PluginSpec::State::Error;
        WARN << (spec.reason = loader.errorString());
    }
    return success;
}

void NativePluginProvider::loadEnabledPlugins()
{
    DEBG << "Loading enabled user plugins…";
    for (auto &[id, spec] : plugins_)
        if (spec.type == PluginSpec::Type::User && isEnabled(id))
            load(spec);
}




//    // ALL plugins
//    std::vector<std::unique_ptr<NativePluginSpec>> plugins_;
//    std::map<QString,NativePluginSpec*> plugin_index;
//    // VALID plugins
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependencies;
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependees;
//
//if (!spec.binary_dependencies.isEmpty())
//for (auto &executable : spec.binary_dependencies)
//if (QStandardPaths::findExecutable(executable).isNull())
//errors << QString("Executable '%s' not found.").arg(executable);
//


//
//void PluginProvider::setEnabled(const QString &id, bool enable)
//{
//    QSettings().setValue(QString("%1/enabled").arg(id), enable);
//    enable ? (void)loadPlugin(id) : unloadPlugin(id);
//
////    if (isEnabled(id) == enabled)
////        return false;
//
////    auto *plugin = plugin_index.at(id);
////
////    if (enabled){
////        // Get dependencies
////        auto dependencies = transitive_dependencies.at(plugin);
////
////        // Remove dependencies which are already enabled
////        for (auto it = dependencies.begin(); it != dependencies.end();)
////            if (isEnabled((*it)->id()))
////                it = dependencies.erase(it);
////            else
////                ++it;
////
////        // Don't break expectations. ask user if plugins get enabled implicitly
////        if (!dependencies.empty()) {
////            QStringList names;
////            for (auto *dep: dependencies)
////                names << dep->metadata().name;
////            auto text = QString("This will also enable %1. Continue?").arg(names.join(","));
////            if (QMessageBox::question(nullptr, text, "Continue?") == QMessageBox::No)
////                return false;
////        }
////
////        dependencies.insert(plugin);
////
////        // Store enabled settings
////        QSettings s(QCoreApplication::instance()->applicationName());
////        for (auto *p : dependencies)
////            s.setValue(QString("%1/enabled").arg(p->id()), true);
////
////        // Safe load order loading
////        for (auto &p : plugins_)
////            if (dependencies.count(p.get()))
////                plugin->load();
////    }
////    else
////    {
////        // Get dependees
////        auto dependees = transitive_dependees.at(plugin);
////
////        // Remove dependees which are already disabled
////        for (auto it = dependees.begin(); it != dependees.end();)
////            if (!isEnabled((*it)->id()))
////                it = dependees.erase(it);
////            else
////                ++it;
////
////        // Don't break expectations. ask user if plugins get disabled implicitly
////        if (!dependees.empty()) {
////            QStringList names;
////            for (auto *dep: dependees)
////                names << dep->metadata().name;
////            auto text = QString("This will also disable %1. Continue?").arg(names.join(","));
////            if (QMessageBox::question(nullptr, text, "Continue?") == QMessageBox::No)
////                return false;
////        }
////
////        dependees.insert(plugin);
////
////        // Store enabled settings
////        QSettings s(QCoreApplication::instance()->applicationName());
////        for (auto *p : dependees)
////            s.setValue(QString("%1/enabled").arg(p->id()), false);
////
////        // Safe load order UN-loading
////        for (auto it = plugins_.crbegin(); it != plugins_.crend(); ++it)
////            if (dependees.count((*it).get()))
////                plugin->load();
////    }
////    return true;
//}
//




//    /*
//     * Topological sort with cycle detection
//     * DO NOT MODIFY! ORDER OF EXECUTION IS CRUCIAL!
//     */
//
//    // Build dependency index of valid plugins
//    map<QString,set<QString>> plugin_dependencies;
//    for (const auto &[id, plugin] : plugins_by_id) {
//        if (plugin->state() != PluginSpec::State::Invalid) {
//            plugin_dependencies[plugin->id()];
//            for (const QString &dependency_id: plugin->metadata().plugin_dependencies) {
//                plugin_dependencies[plugin->id()].insert(dependency_id);
//            }
//        }
//    }
//
//    for (auto it = plugin_dependencies.cbegin(); it != plugin_dependencies.cend();) {
//        const auto&[id, dependency_ids] = *it;
//
//        if (dependency_ids.empty()) {
//            // Move plugin to final topological sorted plugin list
//            plugins_.emplace_back(move(plugins_by_id.at(id)));
//
//            // Remove this from other's dependencies
//            for (auto &plugin_dependency: plugin_dependencies)
//                plugin_dependency.second.erase(id);
//
//            // Remove plugin
//            plugin_dependencies.erase(it);
//
//            // start over
//            it = plugin_dependencies.begin();
//        } else
//            ++it;
//    }
//
//    // The remainder is invalid. Either missing dependencies or dependency cycle. Add though for info.
//
//    for (const auto&[id, dependency_ids] : plugin_dependencies) {
//        QStringList missing;
//        copy_if(dependency_ids.cbegin(), dependency_ids.cend(), inserter(missing, missing.end()),
//                [&](auto &pid){ return plugins_by_id.count(pid) == 0; });
//        if (missing.empty())
//            plugins_by_id.at(id)->setState(PluginSpec::State::Invalid, "Dependency cycle detected.");
//        else
//            plugins_by_id.at(id)->setState(PluginSpec::State::Invalid,
//                                           QString("Missing dependencies").arg(missing.join(", ")));
//        plugins_.emplace_back(move(plugins_by_id.at(id)));
//    }
//
//    // Build an plugin index by id
//    for (auto &plugin : plugins_)
//        plugin_index.emplace(plugin->id(), plugin.get());
//
//    // Build transitive dependency index
//    auto transitive_dependencies_recursion = [&](NativePluginSpec &plugin, set<NativePluginSpec*> &deps){
//        for (auto &dependency : plugin.metadata().plugin_dependencies)
//            deps.insert(plugin_index.at(dependency));
//    };
//
//    // For valid plugins, build dependency lists
//    for (auto &plugin : plugins_)
//        if (plugin->state() != PluginSpec::State::Invalid)
//            transitive_dependencies_recursion(*plugin, transitive_dependencies[plugin.get()]);
//
//    // Invert dependency list (dependee list)
//    for (const auto &[plugin, dependencies] : transitive_dependencies)
//        for (auto &dependency : dependencies)
//            transitive_dependees[dependency].insert(plugin);
//
//    loadFrontend();
//    loadEnabledPlugins();