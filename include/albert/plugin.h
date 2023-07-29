// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/config.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "extension.h"
#include <QObject>
#include <QStringLiteral>

#define EXPAND_STRINGIZE(s) STRINGIZE(s)
#define STRINGIZE(s) #s

///
/// @brief Convenience macro for (incomplete) user property definition.
/// @details Writes the boilerplate code for user properties of extensions.
/// The property this macro produces is _incomplete_. It still requires you
/// to define
///
/// - the public getter 'type name() const'
/// - the private setter 'void set_name_(type)'
///
/// @param type The type of the property
/// @param name The name of the property
/// @param defaultValue The default value of the property
///
#define ALBERT_PLUGIN_PROPERTY_NONTRIVIAL(type, name, defaultValue) \
public: static const type name##_default{defaultValue}; \
protected: void store_##name() { settings()->setValue(EXPAND_STRINGIZE(name), name()); } \
protected: void restore_##name() { set_##name##_(settings()->value(EXPAND_STRINGIZE(name), name##_default).value<type>()); } \
public: void reset_##name() { set_##name##_(name##_default); settings()->remove(EXPAND_STRINGIZE(name));  } \
Q_SIGNAL void name##Changed(); \
Q_PROPERTY(type name READ name WRITE set_##name RESET reset_##name NOTIFY name##Changed USER true) \
public: void set_##name(type val) { if (val != name()){ set_##name##_(val); store_##name(); emit name##Changed(); } } \
private: void set_##name##_(type); \
public: type name() const; \


///
/// @brief Convenience macro for trivial user property definition.
/// @details Writes the boilerplate code for user properties of extensions.
///
/// @param type The type of the property
/// @param name The name of the property
/// @param defaultValue The default value of the property
///
#define ALBERT_PLUGIN_PROPERTY(type, name, defaultValue) \
public: static const type name##_default{defaultValue}; \
protected: void store_##name() { settings()->setValue(EXPAND_STRINGIZE(name), name()); } \
protected: void restore_##name() { name##_ = settings()->value(EXPAND_STRINGIZE(name), name##_default).value<type>(); } \
public: void reset_##name() { name##_ = name##_default; settings()->remove(EXPAND_STRINGIZE(name));  } \
Q_SIGNAL void name##Changed(); \
Q_PROPERTY(type name READ name WRITE set_##name RESET reset_##name NOTIFY name##Changed USER true) \
private: type name##_{defaultValue}; \
public: type name() const { return name##_; } \
public: void set_##name(type val) { if (val != name()){ name##_=val; store_##name(); emit name##Changed(); } } \


///
/// @brief Convenience macro to connect UI elemetens to albert user properties
///
#define ALBERT_PLUGIN_PROPERTY_CONNECT(name, ui_elem, ui_setter, ui_signal) \
ui_elem->ui_setter(name()); \
connect(ui_elem, &std::remove_pointer<decltype(ui_elem)>::type::ui_signal, \
        this, &std::remove_pointer<decltype(this)>::type::set_##name); \
connect(this, &std::remove_pointer<decltype(this)>::type::name##Changed, \
        ui_elem, [=](){ ui_elem->ui_setter(name()); });


///
/// @brief Declare a class as Albert plugin.
///
/// This is a convenience macro that calls the required Qt macro to make a
/// class a QtPlugin with defaults appropiate for an Albert plugin. It uses the
/// global ALBERT_IID define to set the interface id and assumes there is a
/// file named 'metadata.json' located in the root of the plugin source dir
/// containing the metadata as of the Albert plugin. This file may be
/// handcrafted but it is recommended to use the `albert_plugin` CMake macro
/// which automatically generates this file.
///
/// @note This macro has to be put into the plugin class declaration body.
/// The class this macro appears on must be default-constructible, inherit
/// QObject and contain the Q_OBJECT macro. There should be exactly one
/// occurrence of this macro in the source code for a plugin.
///
#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")


namespace albert::plugin
{

///
/// \brief Convenience plugin template class.
/// \details Allows subclassing any QObject derived class and implements
/// virtual functions in PluginInstance using the plugin metadata.
///
/// @note Boolean user properties of registered extensions will be picked
/// up by the 'albert' extension using the Qt metatype system and are provided
/// to the as inline options.
/// @sa ALBERT_PLUGIN_PROPERTY,
///     ALBERT_PLUGIN_PROPERTY_NONTRIVIAL and
///     ALBERT_PLUGIN_PROPERTY_CONNECT.
///
template <class QOBJECT = QObject>
class ALBERT_EXPORT Plugin : public QOBJECT, public albert::PluginInstance {};


///
/// \brief Convenience extension plugin template class.
/// \details Implements virtual functions in Extension using those from
/// PluginInstance, i.e. using the metadata of the plugin.
///
template <class QOBJECT = QObject>
class ALBERT_EXPORT ExtensionPlugin : public albert::plugin::Plugin<QOBJECT>, virtual public albert::Extension
{
public:
    /// @copydoc albert::PluginInstance::id
    QString id() const override { return PluginInstance::id(); }

    /// @copydoc albert::PluginInstance::name
    QString name() const override { return PluginInstance::name(); }

    /// @copydoc albert::PluginInstance::description
    QString description() const override { return PluginInstance::description(); }

    /// @copydoc albert::PluginInstance:extensions
    /// Override returning the plugin itself.
    /// @returns This ExtensionPlugin
    std::vector<Extension*> extensions() override { return {this}; }
};

}
