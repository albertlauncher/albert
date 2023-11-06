// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/config.h"
#include "pluginloaderprivate.h"
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
    // Some python libs do not link against python. Export the python symbols to the main app.
    loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);// | QLibrary::PreventUnloadHint);

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

    if (auto lt = rawMetadata["loadtype"].toString(); lt == "frontend")
        metadata_.load_type = LoadType::Frontend;
    else if(lt == "nounload")
        metadata_.load_type = LoadType::NoUnload;
    else  // "user
        metadata_.load_type = LoadType::User;


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

    if (!errors.isEmpty())
        throw std::runtime_error(errors.join(", ").toUtf8().constData());
}

QtPluginLoader::~QtPluginLoader()
{
    if (state() != PluginState::Unloaded)
        WARN << "Logic error: QtPluginLoader destroyed in non Unloaded state:" << metadata_.id;
}

PluginInstance *QtPluginLoader::instance() const { return instance_; }

const PluginProvider &QtPluginLoader::provider() const { return provider_; }

const PluginMetaData &QtPluginLoader::metaData() const { return metadata_; }

QString QtPluginLoader::load()
{
    if (auto *instance = loader.instance()){
        if ((instance_ = dynamic_cast<PluginInstance*>(instance))){
            return {};
        } else
            return "Plugin is not of type albert::PluginInstance.";
    } else
        return loader.errorString();
}

QString QtPluginLoader::loadUnregistered(ExtensionRegistry *registry, bool load)
{
    if (load)
        return d->load(registry);
    else
        return d->unload(registry);
}

QString QtPluginLoader::unload()
{
    delete instance_;
    instance_ = nullptr;
    return loader.unload() ? QString{} : loader.errorString();
}
