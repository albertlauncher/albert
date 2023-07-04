// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/config.h"
#include "albert/logging.h"
#include "qtpluginloader.h"
#include <QPluginLoader>
using namespace std;
using namespace albert;


QtPluginLoader::QtPluginLoader(QtPluginProvider *provider, const QString &p)
    : PluginLoader(p), provider_(provider)
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

    static const QRegularExpression regex_iid = QRegularExpression(R"R(org.albert.PluginInterface/(\d+).(\d+))R");
    if (auto iid_match = regex_iid.match(metadata_.iid); !iid_match.hasMatch())
        errors << QString("Invalid IID pattern: '%1'. Expected '%2'.")
                      .arg(iid_match.captured(), iid_match.regularExpression().pattern());
    else if (auto plugin_iid_major = iid_match.captured(1).toUInt(); plugin_iid_major != ALBERT_VERSION_MAJOR)
            errors << QString("Incompatible major version: %1. Expected: %2.")
                          .arg(plugin_iid_major).arg(ALBERT_VERSION_MAJOR);
    else if (auto plugin_iid_minor = iid_match.captured(2).toUInt(); plugin_iid_minor > ALBERT_VERSION_MINOR)
            errors << QString("Incompatible minor version: %1. Supported up to: %2.")
                          .arg(plugin_iid_minor).arg(ALBERT_VERSION_MINOR);

    static const QRegularExpression regex_version = QRegularExpression(R"(^\d+\.\d+$)");
    if (!regex_version.match(metadata_.version).hasMatch())
        errors << "Invalid version scheme. Use '<version>.<patch>'.";

    static const QRegularExpression regex_id = QRegularExpression("[a-z0-9_]");
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

QtPluginLoader::~QtPluginLoader()
{
    if (state_ == PluginState::Loaded)
        QtPluginLoader::unload();
}

PluginInstance *QtPluginLoader::instance() const { return instance_; }

QtPluginProvider *QtPluginLoader::provider() const { return provider_; }

const PluginMetaData &QtPluginLoader::metaData() const { return metadata_; }

void QtPluginLoader::load()
{
    if (state_ == PluginState::Invalid)
        qFatal("Loaded an invalid plugin.");
    else if (state_ == PluginState::Loaded)
        return;

    QPluginLoader loader(path);
    // Some python libs do not link against python. Export the python symbols to the main app.
    loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);// | QLibrary::PreventUnloadHint);
    try {
        if (auto *instance = loader.instance()){
            state_ = PluginLoader::PluginState::Loaded;
            if ((instance_ = dynamic_cast<PluginInstance*>(instance))){
                state_info_.clear();
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
    state_ = PluginLoader::PluginState::Unloaded;
    loader.unload();
}

void QtPluginLoader::unload()
{
    if (state_ == PluginState::Invalid)
        qFatal("Unloaded an invalid plugin.");
    else if (state_ == PluginState::Unloaded)
        return;

    QPluginLoader loader(path);
    delete instance_;  // TODO this saves from segfaults but actually this is QPluginLoaders job
    instance_ = nullptr;
    loader.unload();
    state_info_.clear();
    state_ = PluginState::Unloaded;
}
