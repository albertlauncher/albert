// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <vector>
#include <memory>
class QTranslator;
class QSettings;

class Localization
{
public:
    Localization(const QSettings &settings);
    ~Localization();

    bool isActive() const;

    // Restart reqiured to properly take effect
    void setEnabled(bool enable);
    bool isEnabled() const;

private:
    bool enabled_;
    std::vector<std::unique_ptr<QTranslator>> translators;
};
