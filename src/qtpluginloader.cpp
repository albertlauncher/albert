// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/config.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "albert/util/timeprinter.h"
#include "qtpluginloader.h"
#include "qtpluginprovider.h"
#include <QFutureWatcher>
#include <QPluginLoader>
#include <QtConcurrent>
using namespace std;
using namespace albert;


QtPluginLoader::QtPluginLoader(const QtPluginProvider &provider, const QString &p)
    : PluginLoader(p), loader(p), provider_(provider)
{
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

    static const auto regex_iid = QRegularExpression(R"R(org.albert.PluginInterface/(\d+).(\d+))R");
    if (auto iid_match = regex_iid.match(metadata_.iid); !iid_match.hasMatch())
        errors << QString("Invalid IID pattern: '%1'. Expected '%2'.")
                      .arg(iid_match.captured(), iid_match.regularExpression().pattern());
    else if (auto plugin_iid_major = iid_match.captured(1).toUInt(); plugin_iid_major != ALBERT_VERSION_MAJOR)
            errors << QString("Incompatible major version: %1. Expected: %2.")
                          .arg(plugin_iid_major).arg(ALBERT_VERSION_MAJOR);
    else if (auto plugin_iid_minor = iid_match.captured(2).toUInt(); plugin_iid_minor > ALBERT_VERSION_MINOR)
            errors << QString("Incompatible minor version: %1. Supported up to: %2.")
                          .arg(plugin_iid_minor).arg(ALBERT_VERSION_MINOR);

    static const auto regex_version = QRegularExpression(R"(^\d+\.\d+$)");
    if (!regex_version.match(metadata_.version).hasMatch())
        errors << "Invalid version scheme. Use '<version>.<patch>'.";

    static const auto regex_id = QRegularExpression("[a-z0-9_]");
    if (!regex_id.match(metadata_.id).hasMatch())
        errors << "Invalid plugin id. Use [a-z0-9_].";

    if (metadata_.name.isEmpty())
        errors << "'name' must not be empty.";

    if (metadata_.description.isEmpty())
        errors << "'description' must not be empty.";

    // Finally set state based on errors

    if (errors.isEmpty())
        setState(PluginState::Unloaded);
    else {
        WARN << QString("Plugin invalid: %1. (%2)").arg(errors.join(", "), path);
        setState(PluginState::Invalid, errors.join(", "));
    }

    // Some python libs do not link against python. Export the python symbols to the main app.
    loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);// | QLibrary::PreventUnloadHint);
}

QtPluginLoader::~QtPluginLoader()
{
    Q_ASSERT(state() == PluginState::Unloaded);
}

PluginInstance *QtPluginLoader::instance() const { return instance_; }

const PluginProvider &QtPluginLoader::provider() const { return provider_; }

const PluginMetaData &QtPluginLoader::metaData() const { return metadata_; }

QString QtPluginLoader::load(ExtensionRegistry *registry)
{
    switch (state()) {
    case PluginState::Invalid:
        return QStringLiteral("Plugin is invalid.");
    case PluginState::Loaded:
        return QStringLiteral("Plugin is already loaded.");
    case PluginState::Busy:
        return QStringLiteral("Plugin is currently busy.");
    case PluginState::Unloaded:{
        setState(PluginState::Busy);
        watcher_.disconnect();
        connect(&watcher_, &QFutureWatcher<QString>::finished, this, [this, registry]() {
            if (watcher_.result()){
                TimePrinter tp(QString("[%1 ms] spent initializing plugin '%2'").arg("%1", metadata_.id));
                load_(registry);
            } else
                setState(PluginState::Unloaded, loader.errorString());
        });
        watcher_.setFuture(QtConcurrent::run([this]() -> bool {
            TimePrinter tp(QString("[%1 ms] spent loading plugin '%2'").arg("%1", metadata_.id));
            return loader.load();
        }));
        return {};
    }
    }
}

QString QtPluginLoader::unload(ExtensionRegistry *registry)
{
    switch (state()) {
    case PluginState::Invalid:
        return QStringLiteral("Plugin is invalid.");
    case PluginState::Unloaded:
        return QStringLiteral("Plugin is not loaded.");
    case PluginState::Busy:
        return QStringLiteral("Plugin is currently busy.");
    case PluginState::Loaded:{
        TimePrinter tp(QString("[%1 ms] spent unloading plugin '%2'").arg("%1", metadata_.id));
        unload_(registry);
        return {};
    }
    }
}

void QtPluginLoader::load_(albert::ExtensionRegistry *registry)
{
    setState(PluginState::Busy, QString("Loading '%1'."));

    // Try loading the plugin
    QStringList errors;
    try {
        if (auto *q_instance = loader.instance()){ // implicit load if necessary
            if (auto *p_instance = dynamic_cast<PluginInstance*>(q_instance)){
                p_instance->initialize(registry);
                for (auto *e : p_instance->extensions())
                    registry->add(e);
                instance_ = p_instance;
                setState(PluginState::Loaded);
                return;
            } else
                errors << "Plugin is not of type albert::PluginInstance.";
        } else
            errors << loader.errorString();
    } catch (const exception& e) {
        errors << e.what();
    } catch (...) {
        errors << "Unknown exception while plugin instantiation.";
    }

    // Unload in any case
    if (!loader.unload())
        errors << loader.errorString();

    setState(PluginState::Unloaded, errors.join(" "));
}

void QtPluginLoader::unload_(albert::ExtensionRegistry *registry)
{
    setState(PluginState::Busy);

    for (auto *e : instance()->extensions())
        registry->remove(e);

    instance()->finalize(registry);

    setState(PluginState::Unloaded, loader.unload() ? QString{} : loader.errorString());
}
