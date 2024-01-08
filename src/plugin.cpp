// Copyright (c) 2024 Manuel Schneider

#include "albert/plugin.h"
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
using namespace albert::plugin;
using namespace albert;
using namespace std;

class Plugin::Private
{
public:
    QTranslator translator;
};

Plugin::Plugin() : d(make_unique<Private>())
{
    // i18n is the default see
    // https://doc.qt.io/qt-6/qtlinguist-cmake-qt-add-translations.html#embedding-generated-qm-files-in-resources
    if (d->translator.load(QLocale(), id(), "_", ":/i18n"))
        QCoreApplication::installTranslator(&d->translator);
}

Plugin::~Plugin()
{
    QCoreApplication::removeTranslator(&d->translator);
}

QString ExtensionPlugin::id() const { return PluginInstance::id(); }

QString ExtensionPlugin::name() const { return PluginInstance::name(); }

QString ExtensionPlugin::description() const { return PluginInstance::description(); }

std::vector<Extension*> ExtensionPlugin::extensions() { return {this}; }
