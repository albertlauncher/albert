// Copyright (c) 2022 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "logging.h"
#include "pluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
//#include "settings/settingswindow.h"
#ifdef Q_OS_MAC
#include "macos.h"
#endif
using namespace std;
using namespace albert;
static const char *CFG_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontendId";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static App *instance_ = nullptr;

static QStringList defaultPluginDirs()
{
    QStringList pluginDirs;
#if defined __linux__ || defined __FreeBSD__
    QStringList dirs = {
#if defined MULTIARCH_TUPLE
        QFileInfo("/usr/lib/" MULTIARCH_TUPLE).canonicalFilePath(),
#endif
        QFileInfo("/usr/lib/").canonicalFilePath(),
        QFileInfo("/usr/lib64/").canonicalFilePath(),
        QFileInfo("/usr/local/lib/").canonicalFilePath(),
        QFileInfo("/usr/local/lib64/").canonicalFilePath(),
        QDir::home().filePath(".local/lib/"),
        QDir::home().filePath(".local/lib64/")
    };

    dirs.removeDuplicates();

    for ( const QString& dir : dirs ) {
        QFileInfo fileInfo = QFileInfo(QDir(dir).filePath("albert/plugins"));
        if ( fileInfo.isDir() )
            pluginDirs.push_back(fileInfo.canonicalFilePath());
    }
#elif defined __APPLE__
    // TODO deplopyment?
    pluginDirs.push_back(QDir("../lib").canonicalPath());
#elif defined _WIN32
    qFatal("Not implemented");
#endif
    return pluginDirs;
}


App *App::instance()
{
    return instance_;
}

App::App(const QStringList &additional_plugin_dirs)
{
    if (instance_ != nullptr)
        qFatal("App created twice");
    instance_ = this;

#if defined(Q_OS_MAC)
    setActivationPolicyAccessory();
#endif

    plugin_provider.findPlugins(QStringList(additional_plugin_dirs) << defaultPluginDirs());
    extension_registry.add(&plugin_provider);

    loadFrontend();

    notifyVersionChangeAndFirstRun();

    QObject::connect(&rpc_server, &RPCServer::messageReceived,
                     &rpc_server, [this](const QString &message){ handleSocketMessage(message); });

    //albert::showSettings();
}

void App::showSettings()
{
    if (!settings_window)
        settings_window = new SettingsWindow(*this);
    settings_window->bringToFront();
}

void App::notifyVersionChangeAndFirstRun()
{
    auto settings = QSettings();
    auto current_version = qApp->applicationVersion();
    auto last_used_version = settings.value(CFG_LAST_USED_VERSION).toString();

    if (last_used_version.isNull()){  // First run
        QMessageBox(
                QMessageBox::Warning, "First run",
                "This is the first time you've launched Albert. Albert is plugin based. "
                "You have to enable extension you want to use. "
                "Note that you wont be able to open albert without a hotkey or "
                "tray icon.").exec();
        showSettings();
        settings.setValue(CFG_LAST_USED_VERSION, current_version);
    }
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) ){  // FIXME in first major version
        QMessageBox(QMessageBox::Information, "Major version changed",
                    QString("You are now using Albert %1. The major version changed. "
                            "Some parts of the API might have changed. Check the "
                            "<a href=\"https://albertlauncher.github.io/news/\">news</a>.")
                            .arg(current_version)).exec();
    }
}

void App::loadFrontend()
{
    // Get all specs of type frontend
    std::map<QString, const PluginSpec&> frontends;
    for(const auto &[id, spec] : plugin_provider.plugins())
        if(spec.type == PluginType::Frontend)
            frontends.emplace(id, spec);

    if (frontends.empty())
        qFatal("No frontends found.");

    // Helper function loading frontend extensions
    auto load_frontend = [this](const QString &id) -> Frontend* {
        if (auto *p = plugin_provider.loadPlugin(id); p){
            if (auto *f = dynamic_cast<Frontend*>(p); f)
                return f;
            else
                plugin_provider.unloadPlugin(id);
        }
        return nullptr;  // Loading failed
    };

    // Try loading the configured frontend
    auto cfg_frontend = QSettings().value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    if (!frontends.contains(cfg_frontend))
        WARN << "Configured frontend does not exist: " << cfg_frontend;
    else if (frontend = load_frontend(cfg_frontend); frontend)
        return;
    else
        WARN << "Loading configured frontend failed. Try any other.";

    for (auto &[id, spec] : frontends)
        if (frontend = load_frontend(id); frontend) {
            WARN << QString("Using %1 instead.").arg(id);
            return;
        }

    qFatal("Could not load any frontend.");
}

QWidget *App::createSettingsWindow()
{
    return nullptr;  // Todo
}

QString App::handleSocketMessage(const QString &message)
{
    static std::map<QString, std::function<QString()>> actions = {
//            {"", [&](){
//                QStringList l;
//                for (const auto &[key, value] : actions)
//                    l << key;
//                return l.join(QChar::LineFeed);
//            }},
            {"show", [&](){
                frontend->setVisible(true);
                return "Frontend shown.";
            }},
            {"hide", [&](){
                frontend->setVisible(false);
                return "Frontend hidden.";
            }},
            {"toggle", [&](){
                frontend->toggleVisibility();
                return "Frontend toggled.";
            }},
            {"settings", [&](){
                showSettings();
                return "Settings opened,";
            }},
            {"restart", [&](){
                restart();
                return "Triggered restart.";
            }},
            {"quit", [&](){
                quit();
                return "Triggered quit.";
            }}
    };

    try{
        return actions.at(message)();
    } catch (const out_of_range&) {
        WARN << QString("Received invalid RPC command: %1").arg(message);
        return QString("Invalid RPC command: %1").arg(message);
    }
}

void App::setFrontend(const QString id)
{
    QSettings().setValue(CFG_FRONTEND_ID, id);
}

/*tray*/
//QAction* showAction = new QAction("Show", trayIconMenu.get());
//        trayIconMenu->addAction(showAction);
//
//        QAction* settingsAction = new QAction("Settings", trayIconMenu.get());
//        QObject::connect(settingsAction, &QAction::triggered, [&](){
//            settingsWidget->show();
//            settingsWidget->raise();
//        });
//        trayIconMenu->addAction(settingsAction);
//
//        QAction* docsAction = new QAction("Open docs", trayIconMenu.get());
//        QObject::connect(docsAction, &QAction::triggered, [](){
//            QDesktopServices::openUrl(QUrl("https://albertlauncher.github.io/"));
//        });
//        trayIconMenu->addAction(docsAction);
//
//        trayIconMenu->addSeparator();
//
//        QAction* restartAction = new QAction("Restart", trayIconMenu.get());
//        QObject::connect(restartAction, &QAction::triggered, [](){
//            QStringList cmdline(qApp->arguments());
//            if (QProcess::startDetached(cmdline.takeFirst(), cmdline)){
//                // TODO: Potential race conditions on slow systems
//                INFO << "Restarting application:" << cmdline;
//                qApp->quit();
//            } else {
//                WARN << "Restarting application failed:" << cmdline;
//            }
//        });
//        trayIconMenu->addAction(restartAction);
//
//        QAction* quitAction = new QAction("Quit", trayIconMenu.get());
//        QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
//        trayIconMenu->addAction(quitAction);
//
//        trayIcon->setContextMenu(trayIconMenu.get());

/*CoreQueryHandler*/
//struct CoreQueryHandler : public Core::QueryHandler
//{
//    CoreQueryHandler(const vector<shared_ptr<Item>>& items)
//        : Core::QueryHandler("org.albert"), items(items){}
//
//    void handleQuery(Core::Query * query) const override {
//        for (auto item : items)
//            if (!query->string().isEmpty() && item->text().toLower().startsWith(query->string().toLower()))
//                  query->addMatch(item, static_cast<uint>((query->string().length() / item->text().length()) * numeric_limits<uint>::max()));
//    }
//    vector<shared_ptr<Item>> items;
//};
////void App::addCoreItemProvider()
////{
//////    using namespace albert;
//////    struct CoreQueryHandler : public OfflineItemProvider {
//////        vector<IndexItem> index_items;
//////        QString id() const override { return "albert"; }
//////        vector<IndexItem> items() const override { return index_items;};
//////    };
//////
//////    auto *core_query_handler = new CoreQueryHandler;
//////
//////    core_query_handler->index_items = {
//////            {
//////                    makeSharedStandardItem(
//////                            "albert-preferences", QStringList{":app_icon"}, "Settings", "Open the Albert settings window",
//////                            SharedActionVector{makeFuncAction("Open settings.", [this] { showSettings(); })}
//////                    ),
//////                    {{"Settings",       MAX_SCORE}, {"Preferences", MAX_SCORE}}
//////            },
//////            {
//////                    makeSharedStandardItem(
//////                            "albert-quit", QStringList{":app_icon"}, "Quit Albert", "Quit this application",
//////                            SharedActionVector{makeFuncAction("Quit Albert.", [this] { quit(); })}
//////                    ),
//////                    {{"Quit Albert",    MAX_SCORE}}
//////            },
//////            {
//////                    makeSharedStandardItem(
//////                            "albert-restart", QStringList{":app_icon"}, "Restart Albert", "Restart this application",
//////                            SharedActionVector{makeFuncAction("Restart Albert.", [this] { restart(); })}
//////                    ),
//////                    {{"Restart Albert", MAX_SCORE}}
//////            }
//////    };
//////    registerExtension(core_query_handler);
////}        // Core items
//        coreQueryHandler = make_unique<CoreQueryHandler>(initializer_list<shared_ptr<Item>>{
//            makeStdItem("open-preferences",
//                ":app_icon", "Preferences", "Open the Albert preferences window.",
//                initializer_list<shared_ptr<Action>>{
//                    makeFuncAction("Open preferences.", [=](){ settingsAction->trigger(); })
//                }
//            ),
//            makeStdItem("restart-albert",
//                ":app_icon", "Restart Albert", "Restart this application.",
//                initializer_list<shared_ptr<Action>>{
//                    makeFuncAction("Restart Albert", [=](){ restartAction->trigger(); })
//                }
//            ),
//            makeStdItem("quit-albert",
//                ":app_icon", "Quit Albert", "Quit this application.",
//                initializer_list<shared_ptr<Action>>{
//                    makeFuncAction("Quit Albert", [=](){ quitAction->trigger(); })
//                }
//            )
//        });

