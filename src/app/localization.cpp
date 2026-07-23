// Copyright (C) 2026-2026 Manuel Schneider

#include "app.h"
#include "localization.h"
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>
using namespace albert;
using namespace std;

namespace {
static const char* CFG_L10N_ENABLED = "localization_enabled";
static const bool  DEF_L10N_ENABLED = true;
}

Localization::Localization(const QSettings &settings)
{
    // The lookup is flexible but complex. The translations lookup _could_ be emulated to avoid
    // the runtime overhead of loading it to find out if translations exist at all (QTranslator
    // does not have a static method to check if a translation exists). However, this is not worth
    // the effort and a source of errors. Load the translations and check if any were loaded.

    auto translator = make_unique<QTranslator>();
    available_ = translator->load(QLocale(), qApp->applicationName(), "_", ":/i18n");
    enabled_ = settings.value(CFG_L10N_ENABLED, DEF_L10N_ENABLED).toBool();

    if (available_ && enabled_)
    {
        translators.emplace_back(::move(translator));

        if (translator = make_unique<QTranslator>();
            translator->load(QLocale(), "qtbase", "_",
                             QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
            translators.emplace_back(::move(translator));

        for (const auto &t : translators)
            qApp->installTranslator(t.get());
    }
}

Localization::~Localization()
{
    for (const auto &t : translators)
        qApp->removeTranslator(t.get());
}

bool Localization::isActive() const { return !translators.empty(); }

bool Localization::isAvailable() const { return available_; }

bool Localization::isEnabled() const { return enabled_; }

void Localization::setEnabled(bool enable)
{
    if (enabled_ == enable)
        return;

    enabled_ = enable;
    app().settings()->setValue(CFG_L10N_ENABLED, enabled_);
}
