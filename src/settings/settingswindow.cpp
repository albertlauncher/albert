// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/extension/frontend/frontend.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "app.h"
#include "pluginswidget/pluginswidget.h"
#include "querywidget/querywidget.h"
#include "settingswindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QKeySequenceEdit>
#include <QStandardItemModel>
#include <QStandardPaths>
using namespace std;


class QHotKeyEdit : public QKeySequenceEdit
{
public:
    QHotKeyEdit(Hotkey &hk) : hotkey(hk)
    {
        if (!hotkey.isPlatformSupported())
            hide();
        else
            setKeySequence(hotkey.hotkey());
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress){
            auto *keyEvent = static_cast<QKeyEvent*>(event);

            if (Qt::Key_Shift <= keyEvent->key() && keyEvent->key() <= Qt::Key_ScrollLock)
                return true; // Filter mod keys

            if (auto ckc = keyEvent->keyCombination(); hotkey.setHotkey(ckc))
                setKeySequence(ckc);

            return true;
        }
        return false;
    }

    Hotkey &hotkey;
};


SettingsWindow::SettingsWindow(App &app):
    ui(),
    plugin_widget(new PluginsWidget(app.plugin_registry)),
    query_widget(new QueryWidget(app.query_engine))
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    init_tab_general_hotkey(app);
    init_tab_general_frontends(app);
    init_tab_general_terminals(app);

    ui.tabs->insertTab(ui.tabs->count(), app.frontend->createFrontendConfigWidget(), tr("Window"));
    ui.tabs->insertTab(ui.tabs->count(), plugin_widget.get(), tr("Plugins"));
    ui.tabs->insertTab(ui.tabs->count(), query_widget.get(), tr("Query"));

    insert_tab_about();

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::init_tab_general_hotkey(App &app)
{ ui.formLayout_general->insertRow(0, tr("Hotkey"), new QHotKeyEdit(app.hotkey)); }

void SettingsWindow::init_tab_general_frontends(App &app)
{
    // Populate frontend checkbox
    for (const auto *loader : app.plugin_provider.frontendPlugins()){
        ui.comboBox_frontend->addItem(loader->metaData().name, loader->metaData().id);
        if (loader->metaData().id == app.frontend_plugin->metaData().id)
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }
    connect(ui.comboBox_frontend, &QComboBox::currentIndexChanged, this, [this, &app]() {
        app.setFrontend(ui.comboBox_frontend->currentData().toString());
    });
}

void SettingsWindow::init_tab_general_terminals(App &app)
{
    for (const auto &terminal : app.terminal_provider.terminals()){
        ui.comboBox_term->addItem(terminal->name());
        if (terminal.get() == &app.terminal_provider.terminal())
            ui.comboBox_term->setCurrentIndex(ui.comboBox_term->count()-1);
    }

    connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&app](int index){ app.terminal_provider.setTerminal(index); });
}

void SettingsWindow::insert_tab_about()
{
    const auto add = [](QVBoxLayout *layout, QLabel *label)
    {
        label->setWordWrap(true);
        label->setOpenExternalLinks(true);
        label->setTextFormat(Qt::MarkdownText);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    };

    auto *vl = new QVBoxLayout;

    vl->addStretch(1);

    auto *l = new QLabel();
    l->setPixmap(QIcon(":app_icon").pixmap(64, 64));
    add(vl, l);

    add(vl, new QLabel(QString("<b>%1 v%2</b>").arg(qApp->applicationDisplayName(),
                                                    qApp->applicationVersion())));

    l = new QLabel(tr("Written in C++, powered by [Qt](aboutQt)."));
    connect(l, &QLabel::linkActivated, this, [](const QString &link)
            { if(link == "aboutQt") qApp->aboutQt(); });
    add(vl, l);

    add(vl, new QLabel(tr("Join our community on [Telegram](%1) or [Discord](%2). "
                          "Report bugs on [GitHub](%3).")
                           .arg("https://telegram.me/albert_launcher_community",
                                "https://discord.com/invite/t8G2EkvRZh",
                                "https://github.com/albertlauncher/albert/issues/new/choose")));

    add(vl, new QLabel(tr("If you appreciate Albert, show your support through a "
                          "[donation](%1) or by becoming a [sponsor](%2).")
                           .arg("https://albertlauncher.github.io/donation/",
                                "https://github.com/sponsors/ManuelSchneid3r")));

    vl->addStretch(2);

    auto *widget = new QWidget;

    widget->setLayout(vl);

    ui.tabs->insertTab(ui.tabs->count(), widget, tr("About"));
}

void SettingsWindow::bringToFront(const QString &plugin)
{
    show();
    raise();
    activateWindow();
    if (!plugin.isNull()){
        plugin_widget->tryShowPluginSettings(plugin);
        ui.tabs->setCurrentWidget(plugin_widget.get());
    }
}

void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier){

        if (event->key() == Qt::Key_Escape)
            close();

    } else if (event->modifiers() == Qt::ControlModifier){

        if(event->key() == Qt::Key_W)
            close();

        // Tab navi by number
        else if((int)Qt::Key_1 <= event->key() && event->key() < (int)Qt::Key_1 + ui.tabs->count())
            ui.tabs->setCurrentIndex((int)event->key() - (int)Qt::Key_1);
    }

    QWidget::keyPressEvent(event);
}
