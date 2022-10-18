#pragma once
#include "app.h"
#include "export.h"
#include "extension.h"
#include "extensionregistry.h"
#include <QObject>

namespace albert{

template<class T>
class ALBERT_EXPORT ExtensionWatcher
{
public:
    ExtensionWatcher() : ExtensionWatcher(albert::App::instance()->extensionRegistry()) { }

    ExtensionWatcher(const ExtensionRegistry &extensionRegistry)
    {
        conn_r = QObject::connect(&extensionRegistry, &ExtensionRegistry::extensionRegistered,
                                  [this](Extension *e){ if (T t = dynamic_cast<T>(e)) extensionRegistered(t); });

        conn_u = QObject::connect(&extensionRegistry, &ExtensionRegistry::extensionUnregistered,
                                  [this](Extension *e){ if (T t = dynamic_cast<T>(e)) extensionUnregistered(t); });
    }
    ~ExtensionWatcher()
    {
        QObject::disconnect(conn_r);
        QObject::disconnect(conn_u);
    }

    virtual void onExtensionRegistered(T *) = 0;
    virtual void onExtensionUnregistered(T *) = 0;

private:
    QMetaObject::Connection conn_r;
    QMetaObject::Connection conn_u;
};



}

