// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QCoreApplication>
#include <albert/app.h>
#include <albert/export.h>
#include <albert/extensionregistry.h>
#include <albert/logging.h>

namespace albert
{

template<class T>
class Dependency
{
public:

    inline operator T() const { return dependency_; }
    inline operator bool() const { return dependency_ != nullptr; }
    const T* operator->() const { return dependency_; }
    T* operator->() { return dependency_; }
    const T* get() const { return dependency_; }
    T* get() { return dependency_; }

protected:

    Dependency() = default;
    ~Dependency() = default;

    T *dependency_ = nullptr;
};

///
/// Convenience holder class for plugin hard dependencies.
///
/// Fetches and holds a weak pointer an extension `id` of type `T`. This class is intended to be
/// initialized on plugin constuction without a try-catch block.
///
/// @note Hard dependencies have to be listed in the plugin metadata, such that the plugin loader is
/// able to manage the loading order.
///
/// \ingroup plugin
///
template<class T>
class ALBERT_EXPORT StrongDependency final : public Dependency<T>
{
public:

    ///
    /// Constructs a StrongDependency with `id`.
    ///
    /// @throws std::runtime_error if the dependency is not available or not of type `T`.
    ///
    StrongDependency(QString id)
    {
        try
        {
            this->dependency_ = dynamic_cast<T*>(App::instance().extensionRegistry().extensions().at(id));

            if (!this->dependency_)
                throw std::runtime_error(
                        QCoreApplication::translate(
                            "Dependency",
                            "Extension '%1' is available, but it is not of the expected type."
                        ).arg(id).toStdString());
        }
        catch (const std::out_of_range &)
        {
            throw std::runtime_error(
                        QCoreApplication::translate(
                            "Dependency",
                            "The required extension '%1' is not available."
                        ).arg(id).toStdString());
        }
    }
};


///
/// Convenience holder class for plugin soft dependencies.
///
/// Watches for (de)registration of an extension `id` of type `T`. On (de)registration of the
/// dependency the pointer is updated and the callback called.
///
/// If you use this class you may want to lock a mutex against a query handler or an action.
/// This should not be necessary since the plugin loader and the extension registry must not be
/// accessed while a session query is running. Take care though if you defer the execution of
/// the action using a timer or similar.
///
/// \ingroup plugin
///
template<class T>
class ALBERT_EXPORT WeakDependency final : public Dependency<T>
{
public:

    ///
    /// Constructs a WeakDependency with `id` and callback `on_registered`.
    ///
    explicit WeakDependency(const QString &id, std::function<void(bool)> on_registered = {}):
        callback(on_registered),
        id_(id)
    {
        try {
            this->dependency_ = dynamic_cast<T*>(App::instance().extensionRegistry().extensions().at(id));
            if (!this->dependency_)
                WARN << QStringLiteral("Found '%1' but failed casting to expected type.").arg(id);
        } catch (const std::out_of_range &) { /* okay, optional */ }

        conn_add_ = QObject::connect(&App::instance().extensionRegistry(), &ExtensionRegistry::added,
                                     [this](Extension *e){ onRegistered(e);});

        conn_rem_ = QObject::connect(&App::instance().extensionRegistry(), &ExtensionRegistry::removed,
                                     [this](Extension *e){ onDeregistered(e);});
    }

    ~WeakDependency()
    {
        QObject::disconnect(conn_add_);
        QObject::disconnect(conn_rem_);
    }

    std::function<void(bool)> callback;

private:

    void onRegistered(Extension *e)
    {
        if (e->id() != this->id_)
            return;

        if (!this->dependency_)
        {
            if (auto *d = dynamic_cast<T*>(e); d)
            {
                this->dependency_ = d;
                if (callback)
                    callback(true);
            }
            else
                WARN << QStringLiteral("Failed casting '%1' to expected type.").arg(this->id_);
        }
        else
            CRIT << "WeakDependency already set. Internal logic error?";
    }

    void onDeregistered(Extension *e)
    {

        if (e->id() != this->id_)
            return;

        if (this->dependency_)
        {
            if (auto *d = dynamic_cast<T*>(e); d)
            {
                if (callback)
                    callback(false);  // the dependency should still be usable in the callback
                this->dependency_ = nullptr;
            }
            else
                WARN << QStringLiteral("Failed casting '%1' to expected type.").arg(this->id_);
        }
        else
            CRIT << "WeakDependency already unset. Internal logic error?";
    }

    QMetaObject::Connection conn_add_;
    QMetaObject::Connection conn_rem_;
    QString id_;

};

}
