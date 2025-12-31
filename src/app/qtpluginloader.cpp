// Copyright (c) 2022-2025 Manuel Schneider

#include "config.h"
#include "logging.h"
#include "plugininstance.h"
#include "qtpluginloader.h"
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QPluginLoader>
#include <QTranslator>
#include <QtConcurrentRun>
#include <chrono>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std::chrono;
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

    auto iid = loader_.metaData().value("IID"_L1).toString();

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

    if (auto plugin_iid_major = iid_match.captured(1).toUInt();
        plugin_iid_major != ALBERT_VERSION_MAJOR)
    {
        auto msg = QCoreApplication::translate(
            "QtPluginLoader", "Incompatible major version: %1. Expected: %2.");
        msg = msg.arg(iid_match.captured(), iid_match.regularExpression().pattern());
        throw runtime_error(msg.toStdString());
    }

    if (auto plugin_iid_minor = iid_match.captured(2).toUInt();
        plugin_iid_minor > ALBERT_VERSION_MINOR)
    {
        auto msg = QCoreApplication::translate(
            "QtPluginLoader", "Incompatible minor version: %1. Supported up to: %2.");
        msg = msg.arg(iid_match.captured(), iid_match.regularExpression().pattern());
        throw runtime_error(msg.toStdString());
    }

    //
    // Extract metadata
    //

    auto rawMetadata = loader_.metaData().value("MetaData"_L1).toObject();

    auto load_type = PluginMetadata::LoadType::User;
    if (auto lts = rawMetadata["loadtype"_L1].toString();
        lts == "frontend"_L1)
        load_type = PluginMetadata::LoadType::Frontend;
    else if (!lts.isEmpty() && lts != "user"_L1)
        WARN << QString("Invalid load type '%1'. Default to 'user'.").arg(lts);

    metadata_ = albert::PluginMetadata
    {
        .iid                  = iid,
        .id                   = rawMetadata["id"_L1].toString(),
        .version              = rawMetadata["version"_L1].toString(),
        .name                 = fetchLocalizedMetadata(rawMetadata, "name"_L1),
        .description          = fetchLocalizedMetadata(rawMetadata, "description"_L1),
        .license              = rawMetadata["license"_L1].toString(),
        .url                  = rawMetadata["url"_L1].toString(),
        .readme_url           = rawMetadata["readme_url"_L1].toString(),
        .translations         = rawMetadata["translations"_L1].toVariant().toStringList(),
        .authors              = rawMetadata["authors"_L1].toVariant().toStringList(),
        .maintainers          = rawMetadata["maintainers"_L1].toVariant().toStringList(),
        .runtime_dependencies = rawMetadata["runtime_dependencies"_L1].toVariant().toStringList(),
        .binary_dependencies  = rawMetadata["binary_dependencies"_L1].toVariant().toStringList(),
        .plugin_dependencies  = rawMetadata["plugin_dependencies"_L1].toVariant().toStringList(),
        .third_party_credits  = rawMetadata["credits"_L1].toVariant().toStringList(),
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

const PluginMetadata &QtPluginLoader::metadata() const { return metadata_; }

void QtPluginLoader::load()
{
    // Errors are intentionally not logged. Thats the responsibility of the plugin implementation.
    // Plugins are expected to throw a localized message and print english logs using their
    // logging category.

    auto future = QtConcurrent::run([this]
    {
        auto tp_l = system_clock::now();
        if (!loader_.load())
            throw runtime_error(loader_.errorString().toStdString());
        auto dur_l = duration_cast<milliseconds>(system_clock::now() - tp_l).count();

        if (translator = make_unique<QTranslator>();
            translator->load(QLocale(), metadata().id, "_", ":/i18n"))
            DEBG << QString("Using translations for '%1' from %2")
                        .arg(metadata_.id,translator->filePath());
        else
            translator.reset();

        return dur_l;
    })
    .then(this, [this](long long dur_l){
        if (translator)
            QCoreApplication::installTranslator(translator.get());  // Not threadsafe

        auto tp_c = system_clock::now();
        current_loader = this;
        if (auto *instance = loader_.instance();
            !instance)
            throw runtime_error("Plugin instance is null.");
        else if (instance_ = dynamic_cast<PluginInstance*>(instance);
                 !instance_)
            throw runtime_error("Plugin instance is not of type albert::PluginInstance.");
        auto dur_c = duration_cast<milliseconds>(system_clock::now() - tp_c).count();

        emit finished(tr("Loading: %1 ms, Instantiating: %2 ms").arg(dur_l).arg(dur_c));
    })
    .onFailed(this, [this](const QUnhandledException &que) {
        unload();
        if (que.exception())
            try {
                std::rethrow_exception(que.exception());
            } catch (const std::exception &e) {
                emit finished(QString::fromStdString(e.what()));
            }
        else
            emit finished(u"QUnhandledException but exception() returns nullptr"_s);
    })
    .onFailed(this, [this](const std::exception &e) {
        unload();
        emit finished(QString::fromStdString(e.what()));
    })
    .onFailed(this, [this]{
        unload();
        emit finished(u"Unknown exception in QtPluginLoader::load()"_s);
    });
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
        WARN << loader_.errorString();
}

PluginInstance *QtPluginLoader::instance() { return instance_; }
