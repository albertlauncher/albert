// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QSettings>
#define EXPAND_STRINGIZE(s) STRINGIZE(s)
#define STRINGIZE(s) #s

///
/// @brief Convenience macro for (incomplete) user property definition.
///
/// The property this macro produces is _incomplete_. It still requires you
/// to provide definitions for:
///
/// - the public getter '<type> <name>() const'
/// - the private setter 'void set_<name>_(<type>)'
///
/// Expands to (psuedo code):
///
/// - QPROPERTY(…)
/// - <type> <name>_default() {…}
/// - void reset_<name>() {…}
/// - void store_<name>() {…}
/// - void restore_<name>() {…}
/// - void set_<name>(<type>) {…}
/// - signal <name>_changed();
/// - <type> <name>() const;     // Declaration only
/// - void set_<name>_(<type>);  // Declaration only
///
/// @param type The type of the property
/// @param name The name of the property
/// @param defaultValue The default value of the property
/// @param settings Something that evaluates to a  dereferencable QSettings
///        object (*, ->). A pointer, factory, etc.
///
#define ALBERT_PROPERTY_BASE(type, name, defaultValue, settings) \
    public: static type name##_default(){ return defaultValue; }; \
    protected: void store_##name() { settings()->setValue(EXPAND_STRINGIZE(name), name()); } \
    protected: void restore_##name(const auto &s = nullptr) { \
        if (s) set_##name##_(s->value(EXPAND_STRINGIZE(name), name##_default()).template value<type>()); \
        else set_##name##_(settings()->value(EXPAND_STRINGIZE(name), name##_default()).template value<type>()); \
    } \
    public: void reset_##name() { set_##name##_(name##_default()); settings()->remove(EXPAND_STRINGIZE(name));  } \
    Q_SIGNAL void name##_changed(type); \
    Q_PROPERTY(type name READ name WRITE set_##name RESET reset_##name NOTIFY name##_changed USER true) \
    public: void set_##name(type val) { if (val != name()){ set_##name##_(val); store_##name(); emit name##_changed(val); } }


///
/// @brief Convenience macro for (incomplete) plugin user property definition.
///
/// Calls ALBERT_PROPERTY_BASE with PluginInstance::settings.
///
#define ALBERT_PLUGIN_PROPERTY_BASE(type, name, defaultValue) \
    ALBERT_PROPERTY_BASE(type, name, defaultValue, PluginInstance::settings)


///
/// @brief Convenience macro for (incomplete) user property definition.
///
/// Extends ALBERT_PLUGIN_PROPERTY_BASE by (psuedo code):
///
/// - <type> <name>() const;     // Declaration only
/// - void set_<name>_(<type>);  // Declaration only
///
/// The property this macro produces is _incomplete_. It still requires you
/// to provide _definitions_.
///
/// @param type The type of the property
/// @param name The name of the property
/// @param defaultValue The default value of the property
/// @param settings Something that evaluates to a  dereferencable QSettings
///        object (*, ->). A pointer, factory, etc.
///
#define ALBERT_PROPERTY_GETSET(type, name, defaultValue, settings) \
    ALBERT_PROPERTY_BASE(type, name, defaultValue, settings) \
    public: type name() const; \
    private: void set_##name##_(type);


///
/// @brief Convenience macro for (incomplete) plugin user property definition.
///
/// Calls ALBERT_PROPERTY_BASE with PluginInstance::settings.
///
#define ALBERT_PLUGIN_PROPERTY_GETSET(type, name, defaultValue) \
    ALBERT_PROPERTY_GETSET(type, name, defaultValue, PluginInstance::settings)



///
/// @brief Convenience macro for user property definition using a given member.
///
/// Extends ALBERT_PLUGIN_PROPERTY_BASE by (psuedo code):
///
/// - <type> <name>() const {…}
/// - void set_<name>_(<type>) {…}
///
/// @param type The type of the property
/// @param name The name of the property
/// @param member The member to expose as property
/// @param defaultValue The default value of the property
///
#define ALBERT_PROPERTY_MEMBER(type, name, member, defaultValue, settings) \
    ALBERT_PROPERTY_BASE(type, name, defaultValue, settings) \
    public: type name() const { return member; } \
    private: void set_##name##_(type val) { member=val; }

///
/// @brief Convenience macro for plugin user property definition using a given member.
///
/// Calls ALBERT_PROPERTY_MEMBER with PluginInstance::settings.
///
#define ALBERT_PLUGIN_PROPERTY_MEMBER(type, name, member, defaultValue) \
    ALBERT_PROPERTY_MEMBER(type, name, member, defaultValue, PluginInstance::settings)


///
/// @brief Convenience macro for user property definition defining a member.
///
/// Extends ALBERT_PLUGIN_PROPERTY_MEMBER by (psuedo code):
///
/// <type> <name>_;
///
/// @param type The type of the property
/// @param name The name of the property
/// @param defaultValue The default value of the property
///
#define ALBERT_PROPERTY(type, name, defaultValue, settings) \
    ALBERT_PROPERTY_MEMBER(type, name, name##_, defaultValue, settings) \
    private: type name##_{name##_default()};

///
/// @brief Convenience macro for plugin user property definition defining a member.
///
/// Calls ALBERT_PROPERTY with PluginInstance::settings.
///
#define ALBERT_PLUGIN_PROPERTY(type, name, defaultValue) \
    ALBERT_PROPERTY(type, name, defaultValue, PluginInstance::settings)


///
/// @brief Convenience macro to connect UI elemetens to albert user properties
///
/// @param object The object containing the property
/// @param name The property name
/// @param widget The widget pointer to connect to
/// @param widget_setter The setter function of the widget
/// @param widget_signal The changed signal of the widget
///
#define ALBERT_PROPERTY_CONNECT(object, name, widget, widget_setter, widget_signal) \
    widget->widget_setter(object->name()); \
    connect(widget, &std::remove_pointer<decltype(widget)>::type::widget_signal, \
            object, &std::remove_pointer<decltype(object)>::type::set_##name); \
    connect(object, &std::remove_pointer<decltype(object)>::type::name##_changed, \
            widget, [o=object,w=widget](){ w->widget_setter(o->name()); });

///
/// @brief Convenience macro to connect checkboxes to boolean user properties
///
/// Specialization of ALBERT_PLUGIN_PROPERTY_CONNECT
///
/// @param object The object containing the property
/// @param name The property name
/// @param widget The widget pointer to connect to
///
#define ALBERT_PROPERTY_CONNECT_CHECKBOX(object, name, widget) \
ALBERT_PROPERTY_CONNECT(object, name, widget, setChecked, toggled)


///
/// @brief Convenience macro to connect spinboxes to int user properties
///
/// Specialization of ALBERT_PLUGIN_PROPERTY_CONNECT
///
/// @param object The object containing the property
/// @param name The property name
/// @param widget The widget pointer to connect to
///
#define ALBERT_PROPERTY_CONNECT_SPINBOX(object, name, widget) \
ALBERT_PROPERTY_CONNECT(object, name, widget, setValue, valueChanged)
