// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "../export.h"
#include "../extensionregistry.h"
#include <QObject>

namespace albert
{
template<class T>
class ALBERT_EXPORT ExtensionWatcher  /// Non-QObject extension registry observer
{
public:
    explicit ExtensionWatcher(albert::ExtensionRegistry &er) : registry(er)
    {
        conn_r = QObject::connect(&registry, &ExtensionRegistry::added,
                                  [this](Extension *e){ if (T *t = dynamic_cast<T*>(e)) onAdd(t); });

        conn_u = QObject::connect(&registry, &ExtensionRegistry::removed,
                                  [this](Extension *e){ if (T *t = dynamic_cast<T*>(e)) onRem(t); });
    }

    ~ExtensionWatcher()
    {
        QObject::disconnect(conn_r);
        QObject::disconnect(conn_u);
    }

protected:

    virtual void onAdd(T *) {}
    virtual void onRem(T *) {}

    albert::ExtensionRegistry &registry;

private:
    QMetaObject::Connection conn_r;
    QMetaObject::Connection conn_u;
};
}
