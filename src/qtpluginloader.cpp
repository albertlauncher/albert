// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/config.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/logging.h"
#include "qtpluginloader.h"
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QPluginLoader>
#include <QtConcurrent>
using namespace std;
using namespace albert;


static QString fetchLocalizedMetadata(const QJsonObject &json ,const QString &key)
{
    auto locale = QLocale();

    auto k = QStringLiteral("%1[%2]").arg(key, locale.name());
    if (auto v = json[k].toString(); !v.isEmpty())
        return v;

    k = QStringLiteral("%1[%2]").arg(key, QLocale::languageToCode(locale.language()));
    if (auto v = json[k].toString(); !v.isEmpty())
        return v;

    return json[key].toString();
}


QtPluginLoader::QtPluginLoader(const QString &p) : loader_(p), instance_(nullptr)
{
    //
    // Check interface
    //

    metadata_.iid = loader_.metaData()[QStringLiteral("IID")].toString();

    if (metadata_.iid.isEmpty())
        throw runtime_error("Not a Qt plugin");

    static const auto regex_iid = QRegularExpression(R"R(org.albert.PluginInterface/(\d+).(\d+))R");
    auto iid_match = regex_iid.match(metadata_.iid);

    if (!iid_match.hasMatch())
    {
        auto msg = QCoreApplication::translate(
            "QtPluginLoader", "Invalid interface identifier (IID) pattern : '%1'. Expected '%2'.");
        msg = msg.arg(iid_match.captured(), iid_match.regularExpression().pattern());
        throw runtime_error(msg.toStdString());
    }

    if (auto plugin_iid_major = iid_match.captured(1).toUInt(); plugin_iid_major != ALBERT_VERSION_MAJOR)
    {
        auto msg = QCoreApplication::translate(
            "QtPluginLoader", "Incompatible major version: %1. Expected: %2.");
        msg = msg.arg(iid_match.captured(), iid_match.regularExpression().pattern());
        throw runtime_error(msg.toStdString());
    }

    if (auto plugin_iid_minor = iid_match.captured(2).toUInt(); plugin_iid_minor > ALBERT_VERSION_MINOR)
    {
        auto msg = QCoreApplication::translate(
            "QtPluginLoader", "Incompatible minor version: %1. Supported up to: %2.");
        msg = msg.arg(iid_match.captured(), iid_match.regularExpression().pattern());
        throw runtime_error(msg.toStdString());
    }

    //
    // Extract metadata
    //

    const QString key_md = QStringLiteral("MetaData");
    const QString key_id = QStringLiteral("id");
    const QString key_version = QStringLiteral("version");
    const QString key_name = QStringLiteral("name");
    const QString key_description = QStringLiteral("description");
    const QString key_license = QStringLiteral("license");
    const QString key_url = QStringLiteral("url");
    const QString key_authors = QStringLiteral("authors");
    const QString key_runtime_dependencies = QStringLiteral("runtime_dependencies");
    const QString key_binary_dependencies = QStringLiteral("binary_dependencies");
    const QString key_plugin_dependencies = QStringLiteral("plugin_dependencies");
    const QString key_credits = QStringLiteral("credits");
    const QString key_load_type = QStringLiteral("loadtype");
    const QString load_type_frontend = QStringLiteral("frontend");
    const QString load_type_user = QStringLiteral("user");

    auto rawMetadata = loader_.metaData()[key_md].toObject();
    metadata_.id = rawMetadata[key_id].toString();
    metadata_.version = rawMetadata[key_version].toString();
    metadata_.name = fetchLocalizedMetadata(rawMetadata, key_name);
    metadata_.description = fetchLocalizedMetadata(rawMetadata, key_description);
    metadata_.license = rawMetadata[key_license].toString();
    metadata_.url = rawMetadata[key_url].toString();
    metadata_.authors = rawMetadata[key_authors].toVariant().toStringList();
    metadata_.runtime_dependencies = rawMetadata[key_runtime_dependencies].toVariant().toStringList();
    metadata_.binary_dependencies = rawMetadata[key_binary_dependencies].toVariant().toStringList();
    metadata_.plugin_dependencies = rawMetadata[key_plugin_dependencies].toVariant().toStringList();
    metadata_.third_party_credits = rawMetadata[key_credits].toVariant().toStringList();

    auto lt = rawMetadata[key_load_type].toString();
    if (lt == load_type_frontend)
        metadata_.load_type = PluginMetaData::LoadType::Frontend;
    else {
        if (!lt.isEmpty() && lt != load_type_user)
            WARN << QString("Invalid load type '%1'. Default to '%2'.").arg(lt, load_type_user);
        metadata_.load_type = PluginMetaData::LoadType::User;
    }

    //
    // Set load hints
    //
    // ExportExternalSymbolsHint:
    // Some python libs do not link against python. Export the python symbols to the main app.
    // (this comment is like 10y old, TODO check if necessary)
    //
    // PreventUnloadHint:
    // To be able to unload we have to make sure that there is no object of this library alive.
    // This is nearly impossible with the current design. Frontends keep queries alive over
    // sessions which then segfault on deletion when the code has been unloaded.
    //
    // TODO: Design something that ensures that no items/actions will be alive when plugins get
    // unloaded. (e.g. Session class, owning queries, injected into frontends when shown).
    //
    // Anyway atm frontends keep queries alive over session, which is just poor design.
    // However not unloading is an easy fix for now and theres more important stuff to do.
    //

    loader_.setLoadHints(QLibrary::ExportExternalSymbolsHint); //  | QLibrary::PreventUnloadHint);
}

QtPluginLoader::~QtPluginLoader()
{
    if (loader_.isLoaded())
    {
        CRIT << "QtPluginLoader destroyed in loaded state:" << metadata_.id;
        QtPluginLoader::unload();
    }
}

QString QtPluginLoader::path() const { return loader_.fileName(); }

const PluginMetaData &QtPluginLoader::metaData() const { return metadata_; }

void QtPluginLoader::load()
{
    QFutureWatcher<void> watcher;
    watcher.setFuture(QtConcurrent::run([this]() {
        if (!loader_.load())
            throw runtime_error(loader_.errorString().toStdString());
    }));

    QEventLoop loop;
    QObject::connect(&watcher, &decltype(watcher)::finished, &loop, &QEventLoop::quit);
    loop.exec();

    try{
        watcher.waitForFinished();
    } catch (const QUnhandledException &e) {
        if (e.exception())
            std::rethrow_exception(e.exception());
        else
            throw;
    }
}

void QtPluginLoader::unload()
{
    instance_ = nullptr;
    if (!loader_.unload())
        throw runtime_error(loader_.errorString().toStdString());
}

PluginInstance *QtPluginLoader::createInstance()
{
    if (loader_.isLoaded())
    {
        if (!instance_)
        {
            auto *instance = loader_.instance();
            instance_ = dynamic_cast<PluginInstance*>(instance);
            if (!instance_)
                throw runtime_error("Plugin instance is not of type albert::PluginInstance.");

        }
        return instance_;
    }
    return {};
}
