// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QCoreApplication>
#include <albert/export.h>
#include <albert/extensionregistry.h>
#include <albert/logging.h>

namespace albert
{

///
/// Base class for StrongDependency and WeakDependency.
/// Client code should not use this class directly.
///
template<class T>
class ALBERT_EXPORT Dependency
{
public:

    inline operator T() const { return dependency; }
    inline operator bool() const { return dependency != nullptr; }
    const T* operator->() const { return dependency; }
    T* operator->() { return dependency; }
    const T* get() const { return dependency; }
    T* get() { return dependency; }

protected:

    T *dependency = nullptr;

    explicit Dependency(ExtensionRegistry &registry, const QString &id)
    {
        try {
            auto *e = registry.extensions().at(id);
            dependency = dynamic_cast<T*>(e);
            if (!dependency)
                WARN << QString("Found '%1' but failed casting to expected type.").arg(id);
        } catch (const std::out_of_range &) {}
    }
};

///
/// Convenience holder class for plugin hard dependencies.
///
/// Fetches and holds the weak pointer to the dependecy.
/// If the dependency is not available, an exception is thrown.
/// This class is intended to be initialized in plugin constructors.
/// @note Hard dependencies have to be listed in the plugin metadata, such that
/// the plugin loader is able to manage the loading order.
///
template<class T>
class ALBERT_EXPORT StrongDependency final : public Dependency<T>
{
public:

    explicit StrongDependency(ExtensionRegistry &registry, const QString &id):
        Dependency<T>(registry, id)
    {
        if (!this->dependency)
        {
            auto m = QCoreApplication::translate(
                        "Dependency",
                        "Required dependency %1 not available.").arg(id);
            throw m;
        }
    }
};

///
/// Convenience holder class for plugin soft dependencies.
///
/// Fetches and holds the weak pointer to the dependecy.
/// On (de)registration of the dependency the pointer is updated and the callback called.
///
/// If you use this class you may want to lock a mutex against a query handler or an action.
/// This should not be necessary since the plugin loader and the extension registry must not be
/// accessed while a session query is running. Take care though if you defer the execution of
/// the action using a timer or similar.
///
template<class T>
class ALBERT_EXPORT WeakDependency final : public Dependency<T>
{
public:

    ///
    /// @brief WeakDependency constructor
    /// @param registry The extension registry
    /// @param id The id of the dependency
    /// @param registeredCallback A callback that is called when the dependency is (un)registered
    ///
    explicit WeakDependency(ExtensionRegistry &registry, const QString &id,
                            std::function<void(bool)> registeredCallback = {}):
        Dependency<T>(registry, id),
        callback(registeredCallback)
    {
        conn_add_ = QObject::connect(&registry, &ExtensionRegistry::added, [this](Extension *e)
        {
            if (e->id() != id_)
                return;

            if (!this->dependency)
            {
                if (auto *d = dynamic_cast<T*>(e); d)
                {
                    this->dependency = d;
                    if (callback)
                        callback(true);
                }
                else
                    WARN << QString("Failed casting '%1' to expected type.").arg(id_);
            }
            else
                WARN << "WeakDependency already set. Internal logic error?";

        });

        conn_rem_ = QObject::connect(&registry, &ExtensionRegistry::removed, [this](Extension *e)
        {
            if (e->id() != id_)
                return;

            if (this->dependency)
            {
                if (auto *d = dynamic_cast<T*>(e); d)
                {
                    if (callback)
                        callback(false);  // the dependency should still be usable in the callback
                    this->dependency = nullptr;
                }
                else
                    WARN << QString("Failed casting '%1' to expected type.").arg(id_);
            }
            else
                WARN << "WeakDependency already unset. Internal logic error?";
        });
    }

    ~WeakDependency()
    {
        QObject::disconnect(conn_add_);
        QObject::disconnect(conn_rem_);
    }

    ///
    /// @brief (De)Registration callback
    /// @param bool true if the dependency was added, false if it was removed
    ///
    std::function<void(bool)> callback;

private:

    QMetaObject::Connection conn_add_;
    QMetaObject::Connection conn_rem_;
    const QString id_;

};

}
