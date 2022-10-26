// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "app.h"
#include "export.h"
#include "extension.h"
#include <QObject>
#include <set>

namespace albert{

template<class T>
class ALBERT_EXPORT ExtensionWatcher
{
public:
    ExtensionWatcher()
    {
        conn_r = QObject::connect(app, &App::extensionRegistered, [this](Extension *e){
            if (T *t = dynamic_cast<T*>(e)){
                registry.insert(t);
                onReg(t);
            }
        });

        conn_u = QObject::connect(app, &App::extensionUnregistered, [this](Extension *e){
            if (T *t = dynamic_cast<T*>(e)){
                registry.erase(t);
                onDereg(t);
            }
        });

        for (auto &[id, e] : app->extensionsOfType<T>())
            registry.insert(e);
    }

    ~ExtensionWatcher()
    {
        QObject::disconnect(conn_r);
        QObject::disconnect(conn_u);
    }

protected:

    virtual void onReg(T *) {}
    virtual void onDereg(T *) {}

    const std::set<T*> &extensions() const { return registry; }

private:
    std::set<T*> registry;
    QMetaObject::Connection conn_r;
    QMetaObject::Connection conn_u;
};



}

