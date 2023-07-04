// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include <QObject>
#include <map>

namespace albert
{
/// The common extension registry
/// Neither threadsafe, nor reentrant.
/// @note Do touch in the GUI thread only.
class ALBERT_EXPORT ExtensionRegistry : public QObject
{
    Q_OBJECT
public:
    void add(Extension *e);  ///< Add extension to the registry
    void remove(Extension *e);  ///< Remove extension from the registry

    /// Get map of all extensions
    const std::map<QString,Extension*> &extensions();

    /// Get map of all extensions of type
    template<typename T> std::map<QString, T*> extensions()
    {
        std::map<QString, T*> results;
        for (auto &[id, extension] : extensions_)
            if (T *t = dynamic_cast<T*>(extension))
                results.emplace(id, t);
        return results;
    }

    /// Get casted extension by id
    template<typename T> T* extension(const QString &id)
    {
        try {
            return dynamic_cast<T*>(extensions_.at(id));
        } catch (const std::out_of_range &) {
            return nullptr;
        }
    }

signals:
    void added(Extension*);
    void removed(Extension*);

private:
    std::map<QString,Extension*> extensions_;
};
}
