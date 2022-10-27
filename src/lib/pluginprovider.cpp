// Copyright (c) 2022 Manuel Schneider

#include "config.h"
#include "logging.h"
#include "pluginprovider.h"
#include <QCoreApplication>
#include <QDirIterator>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
#include <memory>
using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using albert::PluginType;
using albert::PluginState;
using albert::PluginSpec;

static const char *IID_PATTERN = R"R(org.albert.PluginInterface/(\d+).(\d+))R";
albert::PluginSpec *current_spec_in_construction = nullptr;

PluginProvider::PluginProvider()
{

}

PluginProvider::~PluginProvider()
{
    for (auto &[id, spec] : specs)
        unloadPlugin(id);
}

void PluginProvider::findPlugins(const QStringList &paths)
{
    for (auto &[id, spec] : specs)
        unloadPlugin(id);

    specs.clear();

    // Find plugins
    for (auto it = paths.rbegin(); it != paths.rend(); ++it) {
        QDirIterator dirIterator(*it, QDir::Files);
        while (dirIterator.hasNext()) {
            try {
                auto spec = parsePluginMetadata(dirIterator.next());
                if (const auto &[_, success] = specs.emplace(spec.id, spec); !success)
                    WARN << "Plugin id already exists. Skip" << spec.path;
            } catch (const runtime_error &e) {
                WARN << e.what() << dirIterator.filePath();
            }
        }
    }

    loadEnabledPlugins();

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
}

bool PluginProvider::loadPlugin(const QString &id)
{
    auto spec = specs.at(id);

    switch (spec.state) {
        case PluginState::Error:
            [[fallthrough]];
        case PluginState::Unloaded:
        {
            DEBG << "Loading plugin" << spec.id;
            auto start = system_clock::now();
            spec.state = PluginState::Loading;
            emit pluginStateChanged(spec);
            QPluginLoader loader(spec.path);

            // Some python libs do not link against python. Export the python symbols to the main app.
            loader.setLoadHints(QLibrary::ExportExternalSymbolsHint | QLibrary::PreventUnloadHint);

            QString error;
            try {
                current_spec_in_construction = &spec;
                if (QObject *instance = loader.instance()){
                    DEBG << QString("Success [%1ms]").arg(duration_cast<milliseconds>(system_clock::now() - start).count());
                    spec.state = PluginState::Loading;
                    emit pluginStateChanged(spec);
                    return true;
                } else
                    error = loader.errorString();
                current_spec_in_construction = nullptr;
            } catch (const std::exception& e) {
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
            spec.state = PluginState::Loading;
            spec.reason = error;
            emit pluginStateChanged(spec);
            return false;
        }
            // These should never happen
        case PluginState::Loading:
            qFatal("Tried to load a loading plugin.");
        case PluginState::Loaded:
            return true;
    }
}

void PluginProvider::unloadPlugin(const QString &id)
{
    auto spec = specs.at(id);

    switch (spec.state) {
        case PluginState::Loaded:
        {
            DEBG << "Unloading plugin" << spec.id;
            auto start = system_clock::now();
            QPluginLoader loader(spec.path);
            loader.unload();
            DEBG << QString("%1 unloaded in %2 milliseconds").arg(spec.id)
                        .arg(duration_cast<milliseconds>(system_clock::now()-start).count());
            spec.state = PluginState::Unloaded;
            emit pluginStateChanged(spec);
            return;
        }
            // These should never happen
        case PluginState::Error:
            qFatal("Tried to unload an unloaded (state error) plugin.");
        case PluginState::Unloaded:
            break;
        case PluginState::Loading:
            qFatal("Tried to unload a loading plugin.");
    }
}

void PluginProvider::loadEnabledPlugins()
{
    DEBG << "Loading enabled user plugins…";
    for (auto &[id, spec] : specs)
        if (spec.type == PluginType::User && isEnabled(id))
            loadPlugin(id);
}

albert::PluginSpec PluginProvider::parsePluginMetadata(QString path)
{
    QPluginLoader loader(path);
    QStringList errors;
    albert::PluginSpec spec;
    spec.provider = this;
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
        spec.type = PluginType::None;
    else if (string_type == "frontend")
        spec.type = PluginType::Frontend;
    else
        spec.type = PluginType::User;

    if (!QRegularExpression("^\\d+\\.\\d+$").match(spec.version).hasMatch())
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

    if (!spec.binary_dependencies.isEmpty())
        for (auto &executable : spec.binary_dependencies)
            if (QStandardPaths::findExecutable(executable).isNull())
                errors << QString("Executable '%s' not found.").arg(executable);

    // Finally set state based on errors

    if (errors.isEmpty())
        spec.state = PluginState::Unloaded;
    else{
        WARN << errors.join(" ") << path;
        spec.state = PluginState::Error;
        spec.reason = errors.join(" ");
    }

    return spec;
}

// Interfaces

QString PluginProvider::id() const
{
    return "pluginprovider";
}

QIcon PluginProvider::icon()
{
    return QIcon(":cpp");
}

std::map<QString, PluginSpec> &PluginProvider::plugins()
{
    return specs;
}

bool PluginProvider::isEnabled(const QString &id)
{
    return QSettings(qApp->applicationName()).value(QString("%1/enabled").arg(id), false).toBool();
}

void PluginProvider::setEnabled(const QString &id, bool enabled)
{
    QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(id), enabled);
    loadPlugin(id);

//    if (isEnabled(id) == enabled)
//        return false;

//    auto *plugin = plugin_index.at(id);
//
//    if (enabled){
//        // Get dependencies
//        auto dependencies = transitive_dependencies.at(plugin);
//
//        // Remove dependencies which are already enabled
//        for (auto it = dependencies.begin(); it != dependencies.end();)
//            if (isEnabled((*it)->id()))
//                it = dependencies.erase(it);
//            else
//                ++it;
//
//        // Don't break expectations. ask user if plugins get enabled implicitly
//        if (!dependencies.empty()) {
//            QStringList names;
//            for (auto *dep: dependencies)
//                names << dep->metadata().name;
//            auto text = QString("This will also enable %1. Continue?").arg(names.join(","));
//            if (QMessageBox::question(nullptr, text, "Continue?") == QMessageBox::No)
//                return false;
//        }
//
//        dependencies.insert(plugin);
//
//        // Store enabled settings
//        QSettings s(QCoreApplication::instance()->applicationName());
//        for (auto *p : dependencies)
//            s.setValue(QString("%1/enabled").arg(p->id()), true);
//
//        // Safe load order loading
//        for (auto &p : plugins_)
//            if (dependencies.count(p.get()))
//                plugin->load();
//    }
//    else
//    {
//        // Get dependees
//        auto dependees = transitive_dependees.at(plugin);
//
//        // Remove dependees which are already disabled
//        for (auto it = dependees.begin(); it != dependees.end();)
//            if (!isEnabled((*it)->id()))
//                it = dependees.erase(it);
//            else
//                ++it;
//
//        // Don't break expectations. ask user if plugins get disabled implicitly
//        if (!dependees.empty()) {
//            QStringList names;
//            for (auto *dep: dependees)
//                names << dep->metadata().name;
//            auto text = QString("This will also disable %1. Continue?").arg(names.join(","));
//            if (QMessageBox::question(nullptr, text, "Continue?") == QMessageBox::No)
//                return false;
//        }
//
//        dependees.insert(plugin);
//
//        // Store enabled settings
//        QSettings s(QCoreApplication::instance()->applicationName());
//        for (auto *p : dependees)
//            s.setValue(QString("%1/enabled").arg(p->id()), false);
//
//        // Safe load order UN-loading
//        for (auto it = plugins_.crbegin(); it != plugins_.crend(); ++it)
//            if (dependees.count((*it).get()))
//                plugin->load();
//    }
//    return true;
}

