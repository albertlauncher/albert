// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "albert/extensionregistry.h"
#include <QObject>

namespace albert
{

///
/// Non-QObject extension registry observer.
///
/// Convenient tool to observe the extension registry for types you are
/// interested in. Rationale: QObject does neither support templates nor
/// abstract classes also each object must inherit only one QObject.
/// Therefore this class does not use QObject.
///
/// @see ExtensionRegistry
///
template<class T>
class ALBERT_EXPORT ExtensionWatcher
{
public:
    /// ExtensionWatcher constructor
    /// \param registry The extension registry to track. May be set later.
    explicit ExtensionWatcher(ExtensionRegistry *registry = nullptr)
    {
        if (registry)
            setExtensionRegistry(registry);
    }

    virtual ~ExtensionWatcher()
    {
        QObject::disconnect(conn_add);
        QObject::disconnect(conn_rem);
    }

    /// Sets the extension registry to track
    /// \param registry The extension registry to track
    void setExtensionRegistry(ExtensionRegistry *registry)
    {
        QObject::disconnect(conn_add);
        conn_add = QObject::connect(registry, &ExtensionRegistry::added,
                                    [this](Extension *e){ if (T *t = dynamic_cast<T*>(e)) onAdd(t); });

        QObject::disconnect(conn_rem);
        conn_rem = QObject::connect(registry, &ExtensionRegistry::removed,
                                    [this](Extension *e){ if (T *t = dynamic_cast<T*>(e)) onRem(t); });
    }

protected:

    /// Called when an extension has been registered.
    virtual void onAdd(T *) {}

    /// Called when an extension has been deregistered.
    virtual void onRem(T *) {}

private:
    QMetaObject::Connection conn_add;
    QMetaObject::Connection conn_rem;
};
}
