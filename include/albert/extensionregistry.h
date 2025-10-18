// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <albert/extension.h>
#include <map>

namespace albert
{

///
/// The common extension pool.
///
/// Clients can add their extensions, while services can track extensions by
/// listening to the signals added/removed or any particular extension
/// interface using ExtensionWatcher.
///
/// \ingroup plugin
///
class ALBERT_EXPORT ExtensionRegistry : public QObject
{
    Q_OBJECT

public:

    ///
    /// Add extension to the registry
    ///
    bool registerExtension(Extension*);

    ///
    /// Remove extension from the registry
    ///
    void deregisterExtension(Extension*);

    ///
    /// Get map of all registered extensions
    ///
    const std::map<QString,Extension*> &extensions() const;

    ///
    /// Get map of all extensions of type T
    ///
    template<typename T> std::map<QString, T*> extensions() const
    {
        std::map<QString, T*> results;
        for (auto &[id, extension] : extensions_)
            if (T *t = dynamic_cast<T*>(extension))
                results.emplace(id, t);
        return results;
    }

    ///
    /// Get extension by id implicitly dynamic_cast'ed to type T.
    ///
    template<typename T> T* extension(const QString &id)
    {
        try {
            return dynamic_cast<T*>(extensions_.at(id));
        } catch (const std::out_of_range &) {
            return nullptr;
        }
    }

signals:

    ///
    /// Emitted when an extension has been registered.
    ///
    void added(Extension*);

    ///
    /// Emitted when an extension has been deregistered.
    ///
    void removed(Extension*);

private:

    std::map<QString,Extension*> extensions_;
};

}
