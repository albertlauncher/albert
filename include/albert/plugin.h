// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/config.h"
#include "albert/extension.h"
#include "albert/extension/pluginprovider/plugininstance.h"
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
/// @param object The object containing the property
/// @param name The property name
/// @param widget The widget to connect to
/// @param widget_setter The setter function of the widget
/// @param widget_signal The changed signal of the widget
///
#define ALBERT_PLUGIN_PROPERTY_CONNECT(object, name, widget, widget_setter, widget_signal) \
widget->widget_setter(object->name()); \
connect(widget, &std::remove_pointer<decltype(widget)>::type::widget_signal, \
        object, &std::remove_pointer<decltype(object)>::type::set_##name); \
connect(object, &std::remove_pointer<decltype(object)>::type::name##Changed, \
        widget, [o=object,w=widget](){ w->widget_setter(o->name()); });


///
/// @brief Declare a class as Albert plugin.
///
/// This is a convenience macro that calls the required Qt macro to make a
/// class a QtPlugin with defaults appropiate for an Albert plugin. It uses the
/// global ALBERT_IID define to set the interface id and assumes there is a
/// file named 'metadata.json' located in the root of the plugin source dir
/// containing the metadata of the Albert plugin. This file may be handcrafted
/// but it is recommended to use the `albert_plugin` CMake macro which
/// automatically generates the metadata file.
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
/// \brief The entry point class for native plugins.
/// \details Allows subclassing any QObject derived class.
/// \tparam QOBJECT The QObject (subclass) to inherit. Defaults to OObject
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
/// \copydoc ExtensionPluginInstance
/// \tparam QOBJECT The QObject (subclass) to inherit. Defaults to OObject
///
template <class EXTENSION, class QOBJECT = QObject>
class ALBERT_EXPORT ExtensionPlugin : public QOBJECT, virtual public ExtensionPluginInstance<EXTENSION> {};


}
