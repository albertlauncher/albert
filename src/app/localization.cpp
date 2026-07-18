// Copyright (C) 2026-2026 Manuel Schneider

#include "app.h"
#include "localization.h"
#include "logging.h"
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

Localization::Localization(const QSettings &settings) :
    enabled_(settings.value(CFG_L10N_ENABLED, DEF_L10N_ENABLED).toBool())
{
    if (enabled_)
    {
        DEBG << "Loading translations";

        if (auto translator = make_unique<QTranslator>(qApp);
            translator->load(QLocale(), "qtbase", "_",
                             QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
            translators.emplace_back(::move(translator));

        if (auto translator = make_unique<QTranslator>(qApp);
            translator->load(QLocale(), qApp->applicationName(), "_", ":/i18n"))
            translators.emplace_back(::move(translator));

        for (const auto &t : translators)
        {
            DEBG << " -" << t->filePath();
            qApp->installTranslator(t.get());
        }
    }
}

Localization::~Localization()
{
    for (const auto &t : translators)
        qApp->removeTranslator(t.get());

    translators.clear();
}

bool Localization::isActive() const { return !translators.empty(); }

bool Localization::isEnabled() const { return enabled_; }

void Localization::setEnabled(bool enable)
{
    if (enabled_ == enable)
        return;

    enabled_ = enable;
    app().settings()->setValue(CFG_L10N_ENABLED, enabled_);
}
