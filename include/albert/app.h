// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

/// \defgroup core Core API
/// Core classes and functions.

/// \defgroup core_extension Extension
/// Extension system and built-in extension interfaces classes.
/// \ingroup core

/// \defgroup core_plugin Plugin system
/// \ingroup core
/// Classes and functions of the plugin system.

/// \defgroup core_query Query handling
/// \ingroup core
/// Classes and functions of the query system.

/// \defgroup util Utility API
/// Utility classes and helper functions.

/// \defgroup util_plugin Plugin
/// \ingroup util
/// Utility for plugins.

/// \defgroup util_query Query
/// \ingroup util
/// Utility for indexing and query handling.

/// \defgroup util_icon Icon
/// \ingroup util
/// Utility for icon factories.

/// \defgroup util_net Network
/// \ingroup util
/// Network utility.

/// \defgroup util_system System
/// \ingroup util
/// System/Desktop utility.

/// \defgroup util_ui UI
/// \ingroup util
/// UI utility.

#pragma once
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <filesystem>
#include <map>
#include <memory>
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class Extension;
class UsageScoring;

///
/// The public app instance interface.
///
/// \ingroup core
///
class ALBERT_EXPORT App : public QObject
{
    Q_OBJECT
public:
    /// Returns the global app instance.
    static App &instance();

    /// Shows the frontend and optionally sets the text to _input_text_.
    virtual void show(const QString &input_text = {}) = 0;

    /// Shows the settings window and optionally selects the plugin with _plugin_id_.
    virtual void showSettings(QString plugin_id = {}) = 0;

    /// Get map of all registered extensions
    virtual const std::map<QString,Extension*> &extensions() const = 0;

    /// Get map of all extensions of type T
    template<typename T>
    std::map<QString, T*> extensions() const
    {
        std::map<QString, T*> results;
        for (auto &[id, extension] : extensions())
            if (T *t = dynamic_cast<T*>(extension))
                results.emplace(id, t);
        return results;
    }

    /// Get extension by id implicitly dynamic_cast'ed to type T.
    template<typename T>
    T* extension(const QString &id) const
    {
        try {
            return dynamic_cast<T*>(extensions().at(id));
        } catch (const std::out_of_range &) {
            return nullptr;
        }
    }

    ///
    /// Restarts the application.
    ///
    /// This function is thread-safe.
    ///
    static void restart();

    ///
    /// Quits the application.
    ///
    /// This function is thread-safe.
    ///
    static void quit();

    ///
    /// Returns the path to the application config directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &configLocation();

    ///
    /// Returns the path to the application cache directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &cacheLocation();

    ///
    /// Returns the path to the application data directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &dataLocation();

    ///
    /// Returns a `QSettings` object initialized with the application configuration file path.
    ///
    /// As `unique_ptr` for the sake of movability.
    ///
    /// This function is thread-safe.
    ///
    static std::unique_ptr<QSettings> settings();

    ///
    /// Returns a `QSettings` object initialized with the application state file path.
    ///
    /// As `unique_ptr` for the sake of movability.
    ///
    /// This function is thread-safe.
    ///
    static std::unique_ptr<QSettings> state();

signals:

    /// Emitted when an extension has been registered.
    void added(albert::Extension*);

    /// Emitted when an extension has been deregistered.
    void removed(albert::Extension*);

protected:

    App();
    virtual ~App();

};

} // namespace albert
