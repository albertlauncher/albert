// Copyright (c) 2022-2024 Manuel Schneider

#include "config.h"
#include "logging.h"
#include "plugininstance.h"
#include "qtpluginloader.h"
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QPluginLoader>
#include <QTranslator>
#include <QtConcurrentRun>
using namespace albert;
using namespace std;


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

    auto iid = loader_.metaData()[QStringLiteral("IID")].toString();

    if (iid.isEmpty())
        throw runtime_error("Not a Qt plugin");

    static const auto regex_iid = QRegularExpression(R"R(org.albert.PluginInterface/(\d+).(\d+))R");
    auto iid_match = regex_iid.match(iid);

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
    const QString key_translations = QStringLiteral("translations");
    const QString key_authors = QStringLiteral("authors");
    const QString key_runtime_dependencies = QStringLiteral("runtime_dependencies");
    const QString key_binary_dependencies = QStringLiteral("binary_dependencies");
    const QString key_plugin_dependencies = QStringLiteral("plugin_dependencies");
    const QString key_credits = QStringLiteral("credits");
    const QString key_load_type = QStringLiteral("loadtype");
    const QString load_type_frontend = QStringLiteral("frontend");
    const QString load_type_user = QStringLiteral("user");

    auto rawMetadata = loader_.metaData()[key_md].toObject();

    auto load_type = PluginMetaData::LoadType::User;
    if (auto lts = rawMetadata[key_load_type].toString(); lts == load_type_frontend)
        load_type = PluginMetaData::LoadType::Frontend;
    else if (!lts.isEmpty() && lts != load_type_user)
        WARN << QString("Invalid load type '%1'. Default to '%2'.").arg(lts, load_type_user);

    metadata_ = albert::PluginMetaData
    {
        .iid = iid,
        .id = rawMetadata[key_id].toString(),
        .version = rawMetadata[key_version].toString(),
        .name = fetchLocalizedMetadata(rawMetadata, key_name),
        .description = fetchLocalizedMetadata(rawMetadata, key_description),
        .license = rawMetadata[key_license].toString(),
        .url = rawMetadata[key_url].toString(),
        .translations = rawMetadata[key_translations].toVariant().toStringList(),
        .authors = rawMetadata[key_authors].toVariant().toStringList(),
        .runtime_dependencies = rawMetadata[key_runtime_dependencies].toVariant().toStringList(),
        .binary_dependencies = rawMetadata[key_binary_dependencies].toVariant().toStringList(),
        .plugin_dependencies = rawMetadata[key_plugin_dependencies].toVariant().toStringList(),
        .third_party_credits = rawMetadata[key_credits].toVariant().toStringList(),
        .platforms{},
        .load_type = load_type
    };

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
    // Update 2024:
    //
    // Althought the design _does_ handle object lifetime correctly now the app still segfaults
    // when unloading plugins. Probably due to qt internal connection handling. One example that
    // proved to sefault guaranteed is the WeakDependency class whose connections (at least on
    // macos) call into unloaded code although all connections have been properly disconnected.
    //
    // Probably this should be reported as a bug to Qt. But well, … PreventUnload
    //

    loader_.setLoadHints(QLibrary::ExportExternalSymbolsHint | QLibrary::PreventUnloadHint);
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
    watcher.setFuture(QtConcurrent::run([this]{
        if (!loader_.load())
            throw runtime_error(loader_.errorString().toStdString());
    }));

    try{
        QEventLoop loop;
        QObject::connect(&watcher, &decltype(watcher)::finished, &loop, &QEventLoop::quit);
        loop.exec();
        watcher.future().waitForFinished();

        translator = make_unique<QTranslator>();
        if (translator->load(QLocale(), metaData().id, "_", ":/i18n"))
        {
            DEBG << QString("Using translations for '%1' from %2").arg(metadata_.id,
                                                                       translator->filePath());
            QCoreApplication::installTranslator(translator.get());
        }
        else
            translator.reset();
    }
    catch (const QUnhandledException &e)
    {
        if (e.exception())
            std::rethrow_exception(e.exception());
        else {
            CRIT << "QUnhandledException but exception() returns nullptr";
            throw;
        }
    }
    catch (...)
    {
        CRIT << "Unknown exception in QtPluginLoader::load()";
    }
}

void QtPluginLoader::unload()
{
    if (translator)
    {
        QCoreApplication::removeTranslator(translator.get());
        translator.reset();
    }

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
