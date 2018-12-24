// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <QWidget>
#include "plugin.h"
#include "core_globals.h"

#define ALBERT_EXTENSION_IID ALBERT_PLUGIN_IID_PREFIX".extensionv1-alpha"

namespace Core {

struct Private;
class ExtensionManager;
class QueryHandler;
class FallbackProvider;

/**
 * @brief The extension interface
 */
class EXPORT_CORE Extension : public Plugin
{
public:

    Extension(const QString &id);
    ~Extension();

    /**
     * @brief A human readable name of the plugin
     * @return The human readable name
     */
    virtual QString name() const = 0;

    /**
     * @brief The settings widget factory
     * This has to return the widget that is accessible to the user from the
     * albert settings plugin tab. If the return value is a nullptr there will
     * be no settings widget available in the settings.
     * @return The settings widget
     */
    virtual QWidget* widget(QWidget *parent = nullptr) = 0;

protected:

    /**
     * @brief registerFallbackProvider
     */
    void registerQueryHandler(QueryHandler*);

    /**
     * @brief unregisterFallbackProvider
     */
    void unregisterQueryHandler(QueryHandler*);

    /**
     * @brief registerFallbackProvider
     */
    void registerFallbackProvider(FallbackProvider*);

    /**
     * @brief unregisterFallbackProvider
     */
    void unregisterFallbackProvider(FallbackProvider*);

private:

    std::unique_ptr<Private> d;

    static ExtensionManager *extensionManager;
    friend class ExtensionManager;

};

}
