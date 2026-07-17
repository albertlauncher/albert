// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <memory>
#include <vector>
class QSettings;
class QtPluginProvider;
namespace albert::detail { class Frontend; }
namespace albert { class PluginLoader; }

class FrontendRegistry
{
public:
    FrontendRegistry(const QSettings &settings, const QtPluginProvider &plugin_provider);
    ~FrontendRegistry();

    albert::detail::Frontend &frontend();
    std::vector<albert::PluginLoader*> availableFrontends();
    void setFrontend(unsigned int i);

private:
    class Private;
    std::unique_ptr<Private> d;
};
