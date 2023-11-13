// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/frontend/frontend.h"
#include "albert/logging.h"
#include "app.h"
#include "handlerwidget.h"
#include "pluginwidget.h"
#include "qtpluginloader.h"
#include "settingswindow.h"
#include "trayicon.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QKeySequenceEdit>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <usagedatabase.h>
using namespace std;


class QHotKeyEdit : public QKeySequenceEdit
{
public:
    QHotKeyEdit(Hotkey &hk) : hotkey(hk)
    {
        if (!hotkey.isPlatformSupported()){
            setToolTip("This platform does not support hotkeys.");
            setEnabled(false);
        }
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
    ui(), plugin_widget(new PluginWidget(app.plugin_registry))
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    init_tab_general_hotkey(app);
    init_tab_general_trayIcon(app);
    init_tab_general_frontends(app);
    init_tab_general_terminals(app);
    init_tab_general_search(app);
    init_tab_about();

    ui.tabs->insertTab(ui.tabs->count()-1, app.frontend->createFrontendConfigWidget(), "Window");
    ui.tabs->insertTab(ui.tabs->count()-1, new HandlerWidget(app.query_engine, app.extension_registry), "Handlers");
    ui.tabs->insertTab(ui.tabs->count()-1, plugin_widget.get(), "Plugins");

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::init_tab_general_hotkey(App &app)
{ ui.formLayout_general->insertRow(0, "Hotkey", new QHotKeyEdit(app.hotkey)); }

void SettingsWindow::init_tab_general_trayIcon(App &app)
{
    ui.checkBox_showTray->setChecked(app.tray_icon.isVisible());
    QObject::connect(ui.checkBox_showTray, &QCheckBox::toggled,
                     &app.tray_icon, &TrayIcon::setVisible);
}

void SettingsWindow::init_tab_general_frontends(App &app)
{
    // Populate frontend checkbox
    for (const auto *loader : app.plugin_provider.frontendPlugins()){
        ui.comboBox_frontend->addItem(loader->metaData().name, loader->metaData().id);
        if (loader->metaData().id == app.frontend->id())
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

void SettingsWindow::init_tab_general_search(App &app)
{
    ui.slider_decay->setValue((int)(UsageHistory::memoryDecay() * 100));
    QObject::connect(ui.slider_decay, &QSlider::valueChanged, this,
                     [](int val){ UsageHistory::setMemoryDecay((double)val/100.0); });

    ui.checkBox_prioritizePerfectMatch->setChecked(UsageHistory::prioritizePerfectMatch());
    QObject::connect(ui.checkBox_prioritizePerfectMatch, &QCheckBox::toggled, this,
                     [](bool val){ UsageHistory::setPrioritizePerfectMatch(val); });

    ui.checkBox_emptyQuery->setChecked(app.query_engine.runEmptyQuery());
    QObject::connect(ui.checkBox_emptyQuery, &QCheckBox::toggled, this,
                     [&app](bool val){ app.query_engine.setRunEmptyQuery(val); });
}

void SettingsWindow::init_tab_about()
{
    auto open_link = [](const QString &link){
        if( link == "aboutQt" ){
            qApp->aboutQt();
        } else
            QDesktopServices::openUrl(QUrl(link));
    };

    QString about = ui.about_text->text();
    about.replace("___versionstring___", qApp->applicationVersion());
    ui.about_text->setText(about);
    connect(ui.about_text, &QLabel::linkActivated, this, open_link);
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
