// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QObject>
#include <map>

namespace albert
{
class Extension;
class ALBERT_EXPORT ExtensionRegistry : public QObject  /// Neither threadsafe, nor reentrant
{
    Q_OBJECT
public:
    void add(Extension *e);  /// Add extension to the registry
    void remove(Extension *e);  /// Remove extension from the registry

    const std::map<QString,Extension*> &extensions();  /// Get map of all extensions
    template<typename T> std::map<QString, T*> extensions()  /// Get map of all extensions of type
    {
        std::map<QString, T*> results;
        for (auto &[id, extension] : extensions_)
            if (T *t = dynamic_cast<T*>(extension))
                results.emplace(id, t);
        return results;
    }

    template<typename T> T* extension(const QString &id)  /// Get casted extension by id
    {
        try {
            return dynamic_cast<T*>(extensions_.at(id));
        } catch (const std::out_of_range &) {
            return nullptr;
        }
    }

signals:
    void added(albert::Extension*);
    void removed(albert::Extension*);

private:
    std::map<QString,Extension*> extensions_;
};
}
