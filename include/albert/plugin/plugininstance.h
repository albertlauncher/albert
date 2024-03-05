// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#define EXPAND_STRINGIZE(s) STRINGIZE(s)
#define STRINGIZE(s) #s
#include "albert/config.h"
#include "albert/export.h"
#include <QString>
#include <map>
#include <memory>
class QDir;
class QSettings;
class QWidget;

namespace albert
{
class ExtensionRegistry;
class PluginLoader;


///
/// Abstract plugin instance class.
///
/// The base class every plugin has to inherit.
///
class ALBERT_EXPORT PluginInstance
{
public:
    PluginInstance();

    /// The plugin identifier.
    /// Taken from the metadata.
    QString id() const;

    /// The human readable plugin name.
    /// Taken from the metadata.
    QString name() const;

    /// Brief description of the plugin.
    /// Taken from the metadata.
    QString description() const;

    /// The recommended cache location.
    /// Created if necessary.
    QDir cacheDir() const;

    /// The recommended config location.
    /// Created if necessary.
    QDir configDir() const;

    /// The recommended data location.
    /// Created if necessary.
    QDir dataDir() const;

    /// Persistent plugin settings utilizing QSettings.
    /// Configured to use a section titled <plugin-id>.
    std::unique_ptr<QSettings> settings() const;

    /// Persistent plugin state utilizing QSettings.
    /// Configured to use a section titled <plugin-id>.
    /// \since 0.23
    std::unique_ptr<QSettings> state() const;

    /// The initialization function.
    /// \param registry The extension registry.
    /// \param instances The dependencies of the plugin.
    /// \since 0.23
    virtual void initialize(ExtensionRegistry &registry, std::map<QString,PluginInstance*> dependencies);

    /// The finalization function.
    /// \param registry The extension registry.
    /// \since 0.23
    virtual void finalize(ExtensionRegistry &registry);

    /// Config widget factory.
    virtual QWidget *buildConfigWidget();

    /// Global variable used for static dependency injection.
    /// Constructors are nice to have. However Qt plugins enforce default
    /// constructability. This conflicts the desire to have everything necessary
    /// in the constructor, especially the plugin id from the metadata.
    /// This hack emulates constructor injection and should be safe since plugin
    /// instantiation is serialized.
    static PluginLoader *instanciated_loader;

protected:

    virtual ~PluginInstance();

private:

    class Private;
    const std::unique_ptr<Private> d;
};

}

///
/// @brief Declare a class as native Albert plugin.
///
/// Sets the interface identifier to #ALBERT_PLUGIN_IID and uses the metadata
/// file named 'metadata.json' located at CMAKE_CURRENT_SOURCE_DIR.
///
/// @note This macro has to be put into the plugin class body.
/// The class this macro appears on must be default-constructible, inherit
/// QObject and contain the Q_OBJECT macro. There should be exactly one
/// occurrence of this macro in the source code for a plugin.
///
#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")



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

