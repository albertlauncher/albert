// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "export.h"
#include "extensionregistry.h"
#include <QObject>

namespace albert
{
template<class T>
class ALBERT_EXPORT ExtensionWatcher  /// Non-QObject extension registry observer
{
public:
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
    virtual void onAdd(T *) {}
    virtual void onRem(T *) {}

private:
    QMetaObject::Connection conn_add;
    QMetaObject::Connection conn_rem;
};
}
