// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
#include <memory>
#include <vector>
namespace albert { class ExtensionRegistry; }
class QSettings;
class QDir;
class QWidget;

namespace albert
{
class Extension;

class ALBERT_EXPORT PluginInstance
{
public:
    virtual ~PluginInstance();

    virtual QString id() const = 0;  ///< The unique id.
    virtual void initialize(ExtensionRegistry*);  ///< The initialization function.
    virtual void finalize(ExtensionRegistry*);  ///<  The initialization function.
    virtual std::vector<Extension*> extensions();  ///< The extensions this plugin provides.
    virtual QWidget *buildConfigWidget();  ///< Config widget factory.

    std::unique_ptr<QDir> cacheDir() const;  ///< The recommended cache location.
    std::unique_ptr<QDir> configDir() const;  ///< The recommended config location.
    std::unique_ptr<QDir> dataDir() const;  ///< The recommended data location.
    std::unique_ptr<QSettings> settings() const;  ///< Prepared settings object.
};

}
