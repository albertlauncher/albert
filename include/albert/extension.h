// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "export.h"
#include <QObject>
#include <QString>
#include <QSettings>
#define EXPAND_STRINGIZE(s) STRINGIZE(s)
#define STRINGIZE(s) #s

/// Convenience macro for user property definition.
/// Defines store_*/restore_*/reset_* methods, a *Changed signal and a static const default member variable
/// Declares Q_PROPERTY,
/// @note Requires you to declare getter and setter methods.
#define ALBERT_EXTENSION_PROPERTY_(type, name, default_) \
    Q_PROPERTY(type name READ name WRITE set_##name RESET reset_##name NOTIFY name##Changed USER true) \
    Q_SIGNAL void name##Changed(); \
    protected: void store_##name() { settings()->setValue(EXPAND_STRINGIZE(name), name()); } \
    protected: void restore_##name() { set_##name(settings()->value(EXPAND_STRINGIZE(name), name##Default).value<type>()); } \
    public: void reset_##name() { set_##name(name##Default); settings()->remove(EXPAND_STRINGIZE(name));  } \
    public: static const type name##Default{default_}; \

/**
Convenience macro for user property definition.
Declares public getter <name>() and private setter set_<name>_(type)
methods you have to implement. Defines a public setter that calls your
private setter implementation, stores the property and emits the changed
signal, but only if a change really happened.
@see ALBERT_EXTENSION_PROPERTY_
*/
#define ALBERT_NONTRIVIAL_EXTENSION_PROPERTY(type, name, default_) \
    ALBERT_EXTENSION_PROPERTY_(type, name, default_) \
    public: type name() const; \
    private: void set_##name##_(type); \
    public: void set_##name(type val) { if (val != name()){ set_##name##_(val); store_##name(); emit name##Changed(); } }

/// Convenience macro for user property definition.
/// Additionally defines a member variable and public getter/setter methods
/// @see ALBERT_EXTENSION_PROPERTY_
#define ALBERT_EXTENSION_PROPERTY(type, name, default_) \
    ALBERT_EXTENSION_PROPERTY_(type, name, default_) \
    private: type name##_{name##Default}; \
    public: type name() const { return name##_; } \
    public: void set_##name(type val) { if (val != name()){ name##_ = val; store_##name(); emit name##Changed(); } }

/// Convenience macro to connect UI elemetens to albert user properties
#define ALBERT_CONNECT_EXTENSION_PROPERTY(name, ui_elem, ui_setter, ui_signal) \
    ui_elem->ui_setter(name()); \
    connect(ui_elem, &std::remove_pointer<decltype(ui_elem)>::type::ui_signal, this, &std::remove_pointer<decltype(this)>::type::set_##name); \
    connect(this, &std::remove_pointer<decltype(this)>::type::name##Changed, ui_elem, [=](){ ui_elem->ui_setter(name()); });

namespace albert
{

/// Interface for objects of the extension system
class ALBERT_EXPORT Extension
{
public:
    virtual ~Extension() = default;
    virtual QString id() const = 0;  ///< The guid of the extension
    virtual QString name() const = 0;  ///< Pretty, human readable name
    virtual QString description() const = 0;  ///< Brief description of what this extension provides

    /// The extensions settings
    /// Returns a QSettings object with the group set to the id of the extension.
    std::unique_ptr<QSettings> settings() const;
};


}

