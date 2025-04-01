v0.27.7 (2025-04-02)

Hotfix git submodule urls.

v0.27.6 (2025-04-02)

The plugins are now organized in [submodules](https://github.com/albertlauncher/albert/tree/main/plugins).

## Albert

- Run the empty query on `*`
- Add *'Open in terminal'* action to AppQueryHandler

## API

- Add `albert::Action::hide_on_activation`
- Add `albert::PluginInstance::dataLocations`
- Add `albert::show(const QString&)`
- Add standard message box functions modal to the main window.
  - Add `albert::question(…)`
  - Add `albert::information(…)`
  - Add `albert::warning(…)`
  - Add `albert::critical(…)`

## Plugins

### ``bluetooth``

- Add icons for (in)active/(dis)connected items.
- Add completions.

### ``caffeine``

- Polish translations of durations.
- Add natural duration spec (eg 1h1m).
- Fix weird trigger completions.
- Fix default trigger matching.
- Add deactivation notification.
- Use special text ∞ in settings.

### ``clipboard``

- Avoid flicker by using batch add.

### ``system``

- New icon set.
- Update macOS logout command.

### ``timer``

- Remove timers from empty query without hiding the window.

### ``vpn``

- Add icons clearly indicating the state.

### ``websearch``

- New default icon.

### ``widgetsboxmodel`` and ``widgetsboxmodel-qss``

QStyleSheetStyle and its style sheets are a mess to work with and pretty limited.
They may be suitable to style dedicated widgets but not for entire UIs.
This release lays the foundation for a frontend that is not a pain to work with.
The widgetsboxmodel as you know it has been forked to ``widgetsboxmodel-qss``.
But for now it still has the id `widgetsboxmodel` id until the prototype is polished enough.
The id of the new widgetsboxmodel frontend without style sheets is widgetsboxmodel-ng.

- Theme files support.
- Allow multiline input. Shift enter inserts a newline.
- Empahsize the trigger.
- Add context menu to button
- Allow non hiding actions.
- Add a handcrafted, buffered windowframe. Drop glitchy Qt shadow.
- Add option 'Disable input method'.
- Completion and synopsis side by side.
- Handcrafted rounded rects that support gradients.
- Ads settable window properties like colors, margins, sizes and such.
- Put settingsbutton into the input layout. No overlay anymore.
- Always show action to set the _current_ mode first (dark, light).
- Add debug overlay option
- Loads of fixes and mini improvements.

v0.27.5 (2025-03-06)

Fixes.

## Plugins

- **widgetsboxmodel** Fix dark mode detection on Gnome

## Python

- **coingecko** Make sure cache location exists
- **emoji** Make sure cache location exists

v0.27.4 (2025-03-05)

## Albert

- Fix translation file lookup

## Plugins

- **vpn** 5.0
  - Add xdg platform (network manager)
  - Listen to state changes
- Append _en to the plural files

## Python

- **vpn** Archive

v0.27.3 (2025-02-28)

This release fixes Qt 6.2 backward compatibility (Ubuntu 22.04 builds).

v0.27.2 (2025-02-28)

This is a hotfix release for #1517

v0.27.1 (2025-02-27)

## Albert

- Fixes and minor improvementes
- Update translations

## Plugins

- **vpn**
  - Change icons
  - Fix translations
- **python**
  - Redesign the settings widget
  - Allow users to reset the venv
  - Avoid initializing venv on every start

v0.27.0 (2025-02-27)

This is primarily an intermediate release that reverts bad design decisions that make progress difficult.

## API

- `albert`
  - Remove class `ItemsModel`
  - Remove enum `ItemRoles`
  - Remove `openUrl(const QUrl &url)`
  - Remove `ExtensionWatcher<T>`
  - Add class `ResultItem`
  - Add `const ExtensionRegistry &extensionRegistry();`
  - Add `tryCreateDirectory(const filesystem::path&)`
  - `network()`: Return reference
- `albert::ExtensionRegistry`
  - Remove `T* extension<T>(const QString &id)`
- `albert::PluginInstance`
  - Remove `ExtensionRegistry &registry();`
  - Remove `createOrThrow(const QString &path)`
  - Make `cache/config/dataLocation` return filesystem::path
  - Add `extensions()`
- `ExtensionPlugin`
  - Add `ExtensionPlugin::extensions()`
- `albert::MatchConfig`
  - Avoid recurring default separator regex instatiation
  - Change field order
- `albert::Query`
  - Add isActive()
  - Return `vector<ResultItem>` in `matches` and `fallbacks`
  - Return `bool` in `activate*`
  - Remove signal `finished`
  - Add signal `matchesAboutToBeAdded`
  - Add signal `matchesAdded`
  - Add signal `invalidated`
  - Add signal `activeChanged`
- `albert::TriggerQueryHandler`
  - Pass queries as reference
  - `synopsis()` > `synopsis(const QString &query)`
- `albert::GlobalQueryHandler`
  - Pass queries as reference
  - Remove param of `handleEmptyQuery`
- Rename albert/util.h to albert/albert.h

## Plugins

- **python**
  - Add tests.
  - Google Docstring format stub file.
  - Python API v3.0. See changelog in stub file for details.

v0.26.13 (2025-01-06)

Hotfix backward compatibility.

v0.26.12 (2025-01-06)

## Albert

- Fix device dependent pixmap creation
- Avoid usage of deprecated QStyle::standardPixmap

## Plugins

- **applications** Add ghostty

v0.26.11 (2024-12-30)

## Albert

- Add a motivating text in the plugin settings placeholder page

## Plugins

- **timer** Allow h, m, s durations
  - Clean obsolete translations
  - User timer emoji icon
- **caffeine** Fix non-persistent default interval
- **menubar** Do not retain NSRunningApplication forever
  Rather keep its PID for comparison.
- **widgetsboxmodel** Update icon handling
  - Do not upscale icons that are smaller than requested
  - Draw the icon such that it is centered in the icon area
- **chromium** Update bookmark icon
  Also avoid usage of xdg: schemes on other than linux platforms
- **websearch** Change icon sizes which lead to blurry output
- **applications** Change missing terminal link to issues choice

v0.26.10 (2024-12-06)

## Albert

- Add some more precise Match tests
- Hardcode /usr/local/bin into PATH on macos
- Fix Matcher type conversion
- Precompile headers

## Plugins

- **chromium** Avoid warnings on emtpy paths
- **menubar**
  - Properly display glyphs
  - Fix modifier conversion
  - Minor improvements and fixes
- **bluetooth** Add open settings action

v0.26.9 (2024-12-02)

This release contains the latest bug fixes.

- **widgetsboxmodel** Fix broken input history behavior
- **applications** Set ignore_show_in_keys default to true
- **clipboard** Fix typo
- **path** Show PATH in settings and tr synopsis
- **docs** "Fix" mac builds

v0.26.8 (2024-11-18)

## Albert

- Minor improvements around telemetry

## API

- Add variadic ``Matcher::match(…)``

## Plugins

- **python** Interface 2.5
- **applications**
  - Recurse app directories on macos
  - Revert back to command based heuristic to find terminals
- **menubar** Show hotkeys

v0.26.7 (2024-11-07)

This release contains the new plugin menubar and separates the sleep inhibition extension from the system plugin.

## Albert

- CPack drop qt deloy tool
- Update translations
- Parse cli params asap for faster hotkeys

## Plugins

- **menubar** 1.0
- **system** Move sleep inhibition in separate plugin
- **caffeine** Separate from system

v0.26.6 (2024-10-23)

## Albert

- Actually make use of telemetry and send enabled plugins and activated extensions.

- Add context menu to the plugin list
  - En/disable plugin
  - Un/load plugin
  - Option to sort list by checked state

- Improve testing
  - Drop doctest. Use QTest.
  - Drop doctest and use QTest
  - Enable CTest for better CI

## API

- ``albert``
  - Add ``quit()``
  - Add ``restart()``
  - Add ``open(QUrl url)``
  - Add ``open(QString path)``
  - Add ``open(filesystem::path path)``
- ``InputHistory``
  - Constructor now optionally takes a path
- Add colors to logging.h

## Plugins

- **applications**
  - Send telemetry about available terminals
  - Log warning on unsupported terminals
  - Case sensitive desktop ids
  - Fix desktop entry shadowing
  - Add terminal org.gnome.ptyxis

- **bluetooth**
  - Fix warning on language

- **files**
  - Add filebrowser option: Show hidden files
  - Add filebrowser option: Sort case insensitive
  - Add filebrowser option: Show directories first
  - Use QTest

- **widgetsboxmodel**
  - Fix weird end of history behavior

v0.26.5 (2024-10-16)

## Plugins

- Archive **qmlboxmodel**
- **applications**
  - Fix segfaults on Qt 6.8
  - Add option "split camel case"
  - Add option "use acronyms"
  - Add terminals debian-uxterm and com.alacritty.Alacritty
- **python**
  - Do not allow site_import
- **dictionary**
  - Implement FallbackHandler
- **contacts**
  - Implement IndexQueryHandler
  - Index contacts in background
  - Use all available names
  - Add website addresses actions

v0.26.4 (2024-09-24)

This version contains the latest fixes.

## Albert

- ItemIndex: Fix access to moved item. Skip emtpy IndexItems.

## Plugins

- [applications] Search in /System/Cryptexes/App/System/Applications
- [files] Fix f-term action failing on spaces
- [files] Fix xfce4-terminal
- [snippets] Fix typo
- [websearch] Create required data directory on start

## Python

- [syncthing] Use python-syncthing2. python-syncthing is dead.
- [unit_converter] Remove future typehints

v0.26.3 (2024-09-07)

## plugins

- [calculator_qalculate]
  - Add option: Units in global query
  - Add option: Functions in global query
- [python]
  - Proper venv isolation
  - Fix excluding regex breaking aur builds
  - No quotes around logs

v0.26.2 (2024-08-21)

Hotfix docs plugin cluttering output due to nonunique item id

v0.26.1 (2024-08-20)

[albert]
- Albert license v1.1

[plugins]
- [system]
  - Also match the trigger for sleep inhibition
  - Allow changing trigger for sleep inhibition
- [docs]
  - Proper anchor support for all kinds of docsets
  - Add the type to the description
- [files]
  - Add option "index file path"
  - Also fix persistence for option "case senstive file browsers"
- [applications]
  - Sort terminals list by caseinsensitive name

v0.26.0 (2024-08-16)

[albert]

- Give QIcon::fromTheme another try
- API
  - Remove const from GQH::hgq and GQH::heq
  - Drop albert::runTerminal. Moved to applications plugin.
  - Make private property available in subclasses
  - Add getter for plugin dependecies

[plugins]

- [system:10.0]
  - Add inhibit sleep feature
- [docs:3.17] Be more tolerant with anchors
  Macos and Win cant handle anchors. Use it though because it is still better than
  skipping the item entirely.
- [applications:12.0]
  - Move terminal detection here
  - Proper flatpak terminal support
  - Add public interface to run terminals
  - Proper platform abstraction
  - Foundation for xdg-terminal-execute
  - Foundation for URL scheme and mime type handlers

[python]

- [inhibit_sleep] Archive. Moved to system plugin.
- [docker:3.0] Revert to trigger query handling
  Global query handler not applicable, queries take seconds sometimes
- [unit_converter:1.6] Port to API v2
- [jetbrains:2.0]
  - Add Aqua and Writerside
- [tex_to_unicode:1.3] Port to v2.3
  Author is okay with license change. See https://github.com/orgs/albertlauncher/discussions/1391#discussioncomment-10263863

v0.25.0 (2024-08-02)

[albert]
- Simplify MatchConfig
  Hardcode error_tolerance_divisor to 4
  Close #1412

[plugins]
- [chromium:7.1] Minor improvements
  - Avoid initial double indexing
  - Fix status message in settings window
  - Fix warnings on empty paths
- [bluetooth:1.1] Support fuzzy matching
- [urlhandler:5.2] Use albert::openUrl instead of QDesktopServices::openUrl
  QDesktopServices::openUrl fails on wayland
  Close https://github.com/albertlauncher/albert/issues/1414

v0.24.3 (2024-07-09)

[albert]
- Port applications settings to new id
  Hotfix https://github.com/albertlauncher/albert/issues/1412
- Fix telemetry

[plugins]
- [snippets:5.4] Show a snippet preview in description.
  Close #1387

v0.24.2 (2024-07-02)

[albert]
- Add "open terminal here" to app directory items
- Hotfix #1408

[plugins]
- [python:4.5] Update python stub file

[python]
- [GoldenDict:1.5] Remove breaking type hints

v0.24.1 (2024-06-28)

[plugins]
- [python:4.4] Revert back to using pybind submodule (v2.12.0)

v0.24.0 (2024-06-28)

[albert]

- Ignore soft hyphens in lookup strings
- Add TriggersQueryHandler builtin handler
- Drop PluginConfigQueryHandler
- Ignore order of query words
- Do not run fallbacks on empty queries
- Allow unsetting hotkey on backspace
- Move about into general tab
- Use a button for hotkeys such that tab order is usable
- Cache icons in the fallback handler to avoid laggy resize
- Set 700 on albert dirs
- Use same config location and format on all platforms.
- Show message box on errors while loading enabled plugins
- Make openUrl working on wayland by using xdg-open

[API 0.24]

- Loads of changes around the project structure
  - AUTOMOC,UIC,RCC per target
  - Structure sources in folders
  - Flatten headers
  - No paths in core source files (rather lots of include dirs)
  - Finally proper target export such that plugin build in build tree as well as separate projects
  - Add custom target global_lupdate
- CMake
  - albert_plugin(…) modifications
    - Add QT parameter
    - Add I18N_SOURCES parameter
    - SOURCE_FILES > SOURCES
    - I18N_SOURCE_FILES > I18N_SOURCES
    - INCLUDE_DIRECTORIES > INCLUDE
    - LINK_LIBRARIES > LINK
    - Make SOURCES optional. Specify source conventions: include/*.h, src/*.h, src/*.cpp, src/*.mm, src/*.ui and <plugin_id>.qrc
  - Drop METADATA the metadata.json is a mandatory convention now.
  - Drop TS_FILES. Autosource from 'i18n' dir given a naming convention.
    <plugin_id>.ts and <plugin_id>_<ui_language>.ts
  - Add CMake option BUILD_PLUGINS
- General
  - Move Q_OBJECT into ALBERT_PLUGIN macro
  - Remove app functions from API
  - Rename albert.h to util.h
  - albert::networkManager -> albert::network
  - Add convenience classes for plugin interdependencies
  - Allow RankItems to be created using a Match
  - Revert back to per plugin translations. Plugins shall be self contained modules and in principle be packageable in a separate package.
  - Let QtPluginLoader automatically load translations if available.
  - Add finished and total count to translations metadata
  - User per target compile options
  - Add havePasteSupport()
  - Remove openIssueTracker from interface
  - Separate and improve ALBERT_PLUGIN_PROPERTY macros
    - ALBERT_PROPERTY_GETSET
    - ALBERT_PLUGIN_PROPERTY_GETSET
    - ALBERT_PROPERTY_CONNECT_SPINBOX
    - Add param in property changed signal
- PluginInstance
  - Add {cache,config,data}Location. Checks are up to the clients.
  - Add createOrThrow as a utility function for the above functions.
  - Add weak refs for PluginLoader and ExtensionRegistry
  - Drop convenience functinos like id, name, description.
  - Drop initialize/finalize
  - Registering extensions can fail
  - Auto register root extensions
- Changes to icon provider API
  - Add QIcon support
  - Make it free functions
  - Remove caching
  - Returned size can be smaller than requestedSize, but is never larger.
- Query, engine and handlers
  - Handle handler configuration in core (trigger, fuzzy, enabled).
    Remove the getters, have only setters to update plugins.
    - Add TriggerQueryHandler::setUserTrigger
    - Remove TriggerQueryHandler::trigger()
    - Remove TriggerQueryHandler::fuzzyMatching()
  - Do not allow users to disable triggered query handlers.
    This may end up in states where plugins are loaded but actually not used.
    Also some handlers may rely on them to be there, like e.g. the files global
    handler redirects tabs to the triggered handlers.
  - Remove const from handleTriggerQuery
  - Support ignore diacritics
  - Support ignore word order
  - Make Query contextually convertible to QString
  - Unify query interface, no more global- and triggerquery
  - Add parameterizable Matcher/Match class
  - Add dedicated empty query handling
    Empty patterns should match everything. For global queries thats too
    much. For triggered queries it is desired though. Since a lot of global
    query handlers relay the handleTriggerQuery to handleGlobalQuery it is
    not possible to have both. This introduces a dedicated function for
    GlobalQueryHandlers that will be called on empty queries:

[plugins]

- All plugins: Minor fixes and port to API 0.24
- [widgetsboxmodel:7.4] Use QWindow::startSystemMove instead QWidget:move for Wayland Support
- [websearch:9.0] 0.24
  - Add fallback option
  - Add GPT to default engines
  - Add fallback section.
  - Allow inline editing of fallback and trigger withough using the search engine widget.
  - Use matcher for more tolerant queries
  - Complete to trigger instead of name
- [timezones:1.0]
- [timer:1.0]
- [telegram] Archive failed telegram quick access approach
- [path] Rename from 'terminal'
- [system:9.1] System commands update for KDE Plasma 6
- [ssh:8.3] Allow params only in triggered handler
- [sparkle] Archive for now
- [snippets:5.2] Check if paste is supported at all
- [qmlboxmodel] Port
- [python:4.3]
  - Namespace plugin id
  - Compensate the API changes gracefully to defer a breaking API change
  - Ship stub file with the plugin
  - Add buttons for stubfile and user plugin dir
  - API 2.3
    - Deprecate obscure module attached md_id. Use PluginInstance.id.
    - Expose function albert.havePasteSupport
    - Expose class albert.Matcher
    - Expose class albert.Match
    - Expose method albert.TQH.handleTriggerQuery
    - Expose method albert.GQH.handleGlobalQuery
    - albert.PluginInstance:
        - Add read only property id
        - Add read only property name
        - Add read only property description
        - Add instance method registerExtension(…)
        - Add instance method deregisterExtension(…)
        - Deprecate initialize(…). Use __init__(…).
        - Deprecate finalize(…). Use __del__(…).
        - Deprecate __init__ extensions parameter. Use (de)registerExtension(…).
        - Auto(de)register plugin extension. (if isinstance(Plugin, Extension))
    - albert.Notification:
        - Add property title
        - Add property text
        - Add instance method send()
        - Add instance method dismiss()
    - Minor breaking change that is probably not even in use:
        Notification does not display unless send(…) has been called
- [mpris:3.1] Rewrite using xml interface files
- [exprtk] Archive exprtk prototype
- [docs:6.4] Fix XML based docs.
- [docs:6.3] Do not upscale icons
- [docs:6.2] Fix leak on plugin unloading
- [dictionary:3.1] Drop resources, use Dictionary.app icon
- [datetime:5.1] Separate timetzonehandler
- [datetime:5.0] Add "show_date_on_empty_query" option
- [clipboard:3.1]
  - Check if paste is supported at all
  - Use albert::WeakDependency
- [chromium:7.0]
  - Add completion
  - Display bookmark folder
- [bluetooth:1.0] New extension on macos
  - Enable disable Bluetooth
  - Connect to paired devices
- [applications:11.0] Add non localized option on macos
- [applications:10.0]
  - Merge applications_macos and applications_xdg
  - Add completion

[python]
- All plugins: Minor fixes and port to API 2.3
- [zeal:2.0] Add fallback extension
- [wikipedia:2.0] Add fuzzy search support
- [tr:1.6] Check paste support
- [timer] Move to archive
- [syncthing:1.0] Initial prototype
- [goldendict:1.4] Support flatpaks and goldendict-ng
- [emoji:2.2] Check paste support
- Remove albert.pyi
  Ship, install and update with plugin.
  Add python stubfile to ignore files

v0.23.0 (2024-03-03)

[albert]

* i18n
* Make fallback order settable in new query tab.
* Load native plugins threaded.
* Add --no-load cli param
* Use hashmap and avoid exceptions. Twice as fast 🚀
* Add german translation
* Make "Show settings" action the default for plugin items
* [API:v0.23]
  * Change frontend interface design
  * drop extensions() from PluginInstance interface.
    Extensions can now bei registered dynamically at any time.
  * Reduce the plugin system interfaces to the bare minimum
  * Allow hard plugin dependencies.
  * Private destructors for interfaces
  * Refactoring
    * ExtensionRegistry add > registerExtension
    * ExtensionRegistry remove > deregisterExtension
  * Make UI strings in the metadata required.
  * Allow plugins to have public interfaces
  * Revert to authors. Drop maintainers. (plugin metadata)
  * Remove polymorphism in PluginInstance id/name/description
  * Remove dynamic allocation of cache/config/dataDir()
  * Drop template parameter QOBJECT
  * Frontend is not an extension
  * Support localized metadata
  * CMake interface
    * Drop long_description from metadata
    * Add TS_FILES parameter to albert_plugin macro.
    * Revert back to json metadata file again
    * Complete metadata using cmake project details
    * Move Qt::Widgets into the public link interface

[plugins]

* Support i18n
* [python:4.2] Open external links in config labels by default
* [widgetsboxmodel:7.1] Fix animation on linux
* [qmlboxmodel] archive wip
* [widgetsboxmodel:7.0]
  * Dark theme support
  * Themes update
  * Reproducible style (fusion)
  * Fix history search
  * Move persistent window position to state
  * Clear icon cache on hide
  * Archive unlicensed themes
  * Remove "Show fallbacks on empty result" option
  * Drop fonts from themes
* [websearch:8.0]
  * Capital You_T_ube
  * Add Google translate default engine
* [ssh:8.0]
  * Reduce complexity of this overengineered plugin
    * Remove quick connect
    * Remove known hosts
    * Remove file watchers (configs change not that often)
    * remove indexer mutexes
    * remove fuzzy index
* [snippets:5.0]
  * Public extension interface "Add snippet"
 * [qmlboxmodel:3.0]
  * Archived
* [python:4.0]
  * Drop source watches. a plugin provider cant just reload without notifying the plugin registry
  * API 2.2
    - PluginInstance.configWidget supports 'label'
    - __doc__ is not used anymore, since 0.23 drops long_description metadata
    - md_maintainers not used anymore
    - md_authors new optional field
* [dictionary:3.0] Former platform_services
  * Rename plugin platform services to dict
* [clipboard:3.0]
  * use snippets interface
* [applications_macos:5.0]
  * Use KVO to track NSQuery results

[python]

* Reinstate proper licensing
* [virtualbox:1.6] Add info on vboxapi requirement
* [docker:2.0] Show error on conn failure.
* [pomodoro:1.5] Fix notifications
* [pyi] v2.2
* [inhibit_sleep:1.0] Similar to caffeine, theine, amphetamine etc…

v0.22.17 (2023-11-26)

[albert]
* Prepend albert to logging categories, default filter debug
* Remove logging rules cli arguments
  Dont work on some systems and there is QT_LOGGING_RULES for it
* Differentiate terminator terminals suffering bug 660

[plugins]
* [mpris:2.0] Ported

v0.22.16 (2023-11-18)

[albert]
* Remove the visual warning on crashes.
  This ICE error bug is going to stay around for a while.
* Remove autostart option
  This is a feature unnecessary complicated to maintain and perfectly fine
  to delegate entirely to the desktop environment.
* Add "report" RPC
  To get the ability to fetch report from the running instance.

[plugins]
* [python:2.1.0]
  Make plugin major.minor the interface version
  Improve UX while installing dependencies
* [calc:1.4] Threadsafe and aborting calculations
  libcalculate does not mention any thread safety. lock it.
  Use async calculate(…) function and poll query validity to abort
  unnecessary calculations. This does not only save resources, but is
  also required such that the locks above dont introduce stuttering.
* [system:1.8] Dont prompt on gnome session logout
* [app_xdg:1.8] Use Ubuntu gettext domains

[python]
* [bitwarden] 2.0
  Add copy-username action

v0.22.15 (2023-11-08)

[albert]
* Fix missing smooth transform in icon provider
* Add style information to report
* Use X-GNOME-Autostart-Delay
* Add proper unix signal handling using self pipe trick
* Revert printing to logfile
* Give enough time to connect to other instance.

[plugins]
* [system:1.8] Dont prompt on gnome session logout
* [wbm:1.6] Remove unnecessary cast that may introduce segfaults
* [app_xdg:1.8] Use Ubuntu gettext domains

[python]
* [stub] Fix links

v0.22.14 (2023-10-06)

[albert]
* Let RPCServer take care of crash reports. This is a hotfix to remove the recurring crash report on start, if the app is run more than once, e.g. because the session manager restores a session including albert, but albert is also configured to be autostarted.

v0.22.13 (2023-10-05)

[albert]
* Hotfix create missing application paths
* Fix pixmaps path

[plugins]
* [qml] Fix version branching logic

v0.22.12 (2023-10-03)

[albert]
* CI/CD: Fix path in sed expression

[plugins]
* [sparkle] Add macos updater plugin prototype

[python]
* [jetbrains] Add RustRover editor

v0.22.11 (2023-10-03)

[albert]
* Add missing "-executable=" for macdeployqt plugin parameters

[plugins]
* [py] Hotfix: Workaround https://bugreports.qt.io/browse/QTBUG-117153

v0.22.10 (2023-10-03)

[albert]
* CI/CD: Appcast prototype
* Store log in cache dir
* Add loadtype NOUNLOAD
  There are some plugins that dont like to be unloaded (Sparkle, Python).
  Add a mechanism to let plugins prohibit users to unload it at runtime.

[plugins]
* [py] Fix 6.5.2 only QLogCat quirks. Fixes arch builds

v0.22.9 (2023-09-28)

[albert]
* CD: upload on tag
* Revert. NO_SONAME makes troubles on other platforms.

v0.22.8 (2023-09-28)

[albert]
* Hotfix fixing RPM based builds

v0.22.7 (2023-09-27)

[albert]
* Restore 6.2 backward compatibility

v0.22.6 (2023-09-26)

[albert]
* Proper tab navigation in handler widget
* NativePluginProvider: Use absolute file paths.

[plugins]
* [files] Fix "rel. dirpaths of depth 1 have dot prepended" issue
* [docs] Fix recent changes to download urls
* [qalc] Fix tab order

v0.22.5 (2023-09-22)

[albert]
* CMake: On macOS include the macports lookup path
* Fix segfaults on busywait
* Hardcode /usr/local/bin to PATH
* Move last report ts from settings to state
* Add iconlookup in /usr/local/share although not standardized

[plugins]
* [qml] Add hack around lacking DropShadow.samples in Qt <6.4
* [apps_macos] v1.3
    * Find all apps in home dir
    * Keep apps up to date unsing online search
    * Localized app names
    * Add prefpanes
* [docs] Disable build on macOS. Licensing does not allow usage on macOS.
* [files] Add emtpy trash action on macos
* [muparser] Archive muparser. One calculator is enough.
* [qml] Fix shadow clipping
* [qml] Fix clear on hide breaking history search

[python]
* [goldendict] Fix import issue
* [pass] Add OTP feature

v0.22.4 (2023-08-30)

[plugins]
* [docs] Add cache for docset list
* [docs] Use find_program to find brew for ootb cmake config
* [muparser] Use find_program to find brew for ootb cmake config
* [py] Silently skip dirs and files that are no python modules
* [py] iid v2.1: Add config facilities
* [qalcualte] Use find_program to find brew for ootb cmake config
* [qml] Add Cmd/Ctrl+Enter/Return to show actions
* [snippets] Port old snippets

[python]
* [googletrans] Archive. py-googletrans is broken.
* [translators] Add "translators" plugin
* [emoji] Add "Use derived emojis" option
* [dice_roll] iid 2.0

v0.22.3 (2023-08-17)

[albert]
* Dont show version notification before app is fully initialized

v0.22.2 (2023-08-14)

[albert]
* Fix logging filters
* Proper database move

[plugins]
* [ws] Fix websearch breaking users search engines config
* [ws] Fix websearch not applying icon when selected from file dialog

v0.22.1 (2023-08-14)

[albert]
* Freedesktop notification implementation
* Adopt generic Notification interface on macOS
* Fix Linux paste action

[plugins]
* [apps_xdg] Default trigger "apps"
* [yp] Update notification function
* [py] Fix function warn > warning
* [clipboard] Add paste action

[python]
* [pint,yt] Archived. Require maintenance
* [timer] Adopt notification api changes

v0.22.0 (2023-08-12)

### Albert

* Add commandline option for logging filter rules
* Add contour terminal
* Add settingswindow shortcut action for plugin settings
* Add feature copy and paste
* Add "Run empty query" option
* Add handler configurations tab
* Sort fallbacks
* LexSort items having equal score
* Doxygen documentation

API 0.22:

* TriggerQueryHandler
    * Add bool supportsFuzzyMatching()
    * Add bool fuzzyMatchingEnabled()
    * Add void setFuzzyMatchingEnabled(bool)
    * Add QString trigger() (the user configured one)
* GlobalQueryHandler
    * Add applyUsageScore(…).
    * Inherit TQH, i.e. every handler is a TQH
* IndexQueryHandler
    * Reimplement TQH fuzzy methods
    * Default synopsis <filter>
* Plugin system
    * Revert multithreaded plugin laoding (Qt makes problems everywhere)
    * Statically inject metadata, use it for PluginInstances
    * Move native plugin interface into plugin:: namespace
    * Cache/Conf/Data dirs per plugin only (were per Extension)
    * Add PluginInstance::extensions()
    * Add Template based ExtensionPlugin(Instance)
    * Make native plugin a template class to allow subclassing any QObject
* Frontend:
    * Add Frontend::winId, Move the window quirks to the core
    * Use appwide input history file
    * Add generic qml/widgets icon provider to interface
    * Add generic icon provider, creating icons on the fly
* Fuctions and macros:
    * Put all free functions in albert.h
    * Add openUrl QUrl overload
    * Add convenience macros for user property definition
    * Require albert logging category to pass the name
    * Add state file
    * Add global settings factory
* Rename History to InputHistory
* Drop QueryHandler convenience class
* Drop global albert.h include

### Plugins

* [clipboard]
    * Add paste action
* [wbm]
    * Remove option "display icon"
    * Appwide input history
* [websearch]
    * Adopt to sorted fallbacks, drop dragndrop in listview
    * Add drag'n'drop image feature
* [snippets]
    * Add paste action
* [qml:2.0] Revamped QML frontend
* [python]
    * Mimic internal api as close as possible
    * Attach logging functions to plugin modules
    * Expose albert::setClipboardTextAndPaste
    * Expose albert::Notification
* [files]
    * Show filePath instead path in subtext
    * Add option for case sentivity of fs browsers.
    * Add user property for inline config

### Python plugins

* Interface v2.0 stub
* [emoji] New generic and platform agnostic emoji implementation
* [duckduckgo] Add extension
* [color] Add extension

And loads of other changes…

v0.21.1 (2023-06-27)

[albert]
* Add cmd/ctrl + number tab navigation in settings
* Automatically add hpp and qml files to plugin projects

[plugins]
* [docs:1.2] Polish. Fix #106
* Fix misleading comment in config widget

[python]
* [stub] Add missing imports to stub file
* [tex_to_unicode] Fix crash due to wrong type annotation
* [emoji] Fix #179. Call cacheLocation as method of self.

v0.21.0 (2023-06-23)

[albert]
* v0.21.0
  * Remove Item::hasActions
  * Add global config, cache and data location functions
  * Change RankItem::score type to float (0,1]
  * Make queries pointers in handler functions
  * Add function to get global network manager
  * Use explicit named query handling methods (no parameter overloading) `handleTriggerQuery` and `handleGlobalQuery`. This reduces confusion, avoids annoying extra boilerplate to disambiguate methods to avoid hide-virtual warnings and serves as a lowest common denominator on a language/naming level since these features may not be supported by script languages (e.g. Python).
* Settings window
  * Add a new search widget in settingswindow
  * Make handlers of all types optionable
  * Make window and search widgets tabs in the settings window
* Change usagedatabase location to datadir
* Change IPC socket path to $CACHEDIR/albert/ipc_socket. Was $CACHEDIR/albert_socket.
* Fix triggered global query MRU sort

[plugins]
* New extensions:
  * [docs] Reduced set of Zeal docsets at hands
  * [clipboard] Clipboard history
* [contacts:1.2]
  * Formatting: Remove Apple specific braces
* [snip:1.3]
  * Add "Add" and "Remove" button in config widget
  * Add "Add snippet" item on "add" query
  * Add "Remove" action to snippet items
* [python:1.8] Adopt API v0.21. New interface version iid 1.0
  * Add Extension.cache-, config- and dataLocation
  * Expose FallbackHandler
  * Proper multi extension registration
  * Move interface spec into python stub file (yay!)
  * Expose TriggerQueryHandler
  * Expose GlobalQueryHandler
  * Expose QueryHandler
  * Expose IndexQueryHandler
  * Expose Item class entirely such that plugins can subclass it
  * Use pointer for queries
  * Remove global cache/config/data dir functions
* [all] Adopt API v0.21.0

[python]
* Add stub file for type hinting and documentation in IDEs
* New extensions:
  * [coingecko] Crypto currencies
  * [mathematica] iid:1.0 port
* [all] Adopt iid v1.0

v0.20.14 (2023-05-01)

[albert]
* Sort triggerwidget by name rather than id
* Avoid segfaults when setting hotkey failed.

[plugins]
* [ws] fix oversized text in config
* [sys:1.6] Dynamic default commands.
* [app_xdg] Remove content margins of settings widget
* [system] Add lxqt defaults

[python]
* [python_eval:1.3] Fix type of result in item subtext
* [locate:1.7] Fix lambda capture
* remove api_test
* [aur:1.6] Fix install action
* [jetbrains_projects] handle missing projectOpenTimestamp

v0.20.13 (2023-03-30)

[plugins]
* [ws] Show space markers in trigger section
* [vbox:v1.3] Port iid:0.5
* [dice_roll] iid:0.5 v1.0
* [emoji] iid:0.5 v1.0

v0.20.12 (2023-03-29)

[plugins]
* [system:1.4] Make items checkable and titles customizeable

v0.20.11 (2023-03-27)

[albert]
* Respect whitespaces in rpcs

[plugins]
* [wbm] Add option "Center on active screen"
* [app_xdg] Add action "reveal desktop entry"
* [files] Workaround Qt appending slash to root paths

[python]
* [bitwarden] 1.1 (iid: 0.5)
* [vpn] Add wireguard to connection types
* [pacman] Fix out of scope lambda vars

v0.20.10 (2023-03-20)

* [vpn] Add wireguard to connection types
* Several bugfixes

v0.20.9 (2023-03-13)

[albert]
* Update supported terminals (add st and blackbox, remove tilda)

[plugins]
* [wbm] Hide task bar entry
* [ws] Add google scholar to defaults

v0.20.8 (2023-02-11)

[albert]
* Tilda support
* Print font in report

[plugins]
* [contacts_mac] v1.0
* [wbm] Dont hide window when control modifier is hold
* [xdgapps] Do not inherit QT_QPA_PLATFORM to launched apps

v0.20.7 (2023-02-10)

[albert]
* Clear icon cache if unused for a minute.

[plugins]
* [wbm] Postpone query deletion until hide event to prevent busy wait for destruction

v0.20.6 (2023-02-08)

[albert]
* Close settingswindow on ctrl+w

[plugins]
* [wbm] Avoid segfaults on failing screenAt()

[python]
* [jetbrains:1.1] Polish. Fix Macos.

v0.20.5 (2023-02-01)

[albert]
* Drop usage weight. Add option to prioritize perfect matches. See #695

v0.20.4 (2023-01-31)

[albert]
* Reintroduce telemetry
* Fix disfunctional link in settings
[python]
* [tex_to_unicode] py interface 0.5
* [vpn] 1.1 (iid: 0.5)
* [yt] v1.3 create tmp dirs lazily
* [jetbrains] 1.0 (iid:0.5)

v0.20.3 (2023-01-27)

[albert]
* Remove plugin registry from global search
* Add -Wno-inline
* Fix line breaks in errors displayed in settings
* Tray icon isMask

[plugins]
* [platform_mac] 1.0
* [py] Add button to open the dependency dir

[python]
* [pint] 1.0 (currency converter)

v0.20.2 (2023-01-25)

[plugins]
* [py] Quote cd path

[python]
* [pacman] v1.6 iid:0.5
* [timer] v1.4 iid:0.5

v0.20.1 (2023-01-25)

[albert]
* Fix pedantic warnings
* BW tray
* Use env vars to set default locale
* Strech config widget
* Fix segfaults on empty icon name lookup

[plugins]
* Lots of UI polishing
* [qalc] Fix precision probles
* [websearch] Add google maps to defaults
* [datetime] Use default locale

v0.20.0 (2023-01-24)

[albert]
* Config widget per plugin (v0.20)
* Make Triggerwidget edit trigger on double click anywhere

[plugins]
* [chromium:1.4] Add path reset button

[python]
* [locate] 1.6
* [docker] 1.3

Lots of polishing around the new UI

v0.19.4 (2023-01-22)

[plugins]
* [qalc] v1.0 Prototype

v0.19.3 (2023-01-22)

[albert]
* [md] Use content if long description is a file path
* Use both, extension and item id, as icon cache key
* Add standard pixmaps support to iconprovider
* Workaround terminator bug #702

[plugins]
* [wbm] Add Nord theme
* [calc] Respect LC_*
* [chromium] Fix filewatcher does not watch bookmarks
* [wbm] Do not exit on missing themes
* [wbm] Use generic placeholder color for input hint
* [app:xdg] Add exec key option. Also exclude 'env' in exec keys.
* [wbm] Fix clipped label
* [WBM] fix open theme file action
* [files] Provide trash item
* [wbm] Fix list view height margins

[python]
* [trash] Drop. Provided by files plugin now.

v0.19.2 (2023-01-18)

[plugins]
* [datetime] v1.0
* [urlhandler] Fix tld validation

v0.19.1 (2023-01-18)

[albert]
* Fix recurring new version info
* Allow copyconstruction of rank and index items

[plugins]
* [calc] 1.5
  * Inline evaluation
  * Default trigger '='
  * Synopsis
* [wbm] Add item activation using Ctrl+O

v0.19.0 (2023-01-18)

[albert]
* Add reload actinon for plugins
* Support Console term
* Fix backgroundexecutor not using move semantics
* 0.19 interface
  * Revert to dedicated FallbackHandler
  * Clean interface using opaque pointers
  * GlobalQueryHandler::rankItems -> handlyQuery
  * IndexQueryHandlers have to set items directly
* Refactoring
* Show plugin header files in IDEs
* Use handcrafted icon lookup again

[plugins]
* [wbs] 1.3 add query handler providing themes
* [apps_xdg] 1.5 Remove desktop indexing
* [ssh] 1.5
  * Fix ssh connect containing user or port
  * Allow specifying a command to send to the host
  * Add action (keep/close term)

[python]
* [yt] v1.2 (iid:0.5)
* [kill] v1.1 (iid:0.5)

v0.18.13 (2023-01-13)

Fix invalid submodule link breaking OBS builds

[plugins]
* [chromium] Fix config loading

[python]
* [goldendict] 1.1 (0.18)

v0.18.12 (2023-01-13)

[albert]
* Always print report in debug mode
* Add platform, lang and locale to report
* Support Terminology

[plugins]
* [mac_apps] Dons show system service apps
! Add default md_id if not available

v0.18.11 (2023-01-11)

[albert]
* Add missing long description in plugin metadata.

[plugins]
* Handcraft tld validation. Make handler global.
* Add metadata LONG_DESCRIPTON to docs
* [py] Create site-packages dir if necessary
* Fix open snippet

v0.18.10 (2023-01-09)

Fixes, minor changes and requests

v0.18.9 (2023-01-07)

[plugins]
* [py] Ask user to install missing python dependencies in terminal

[python]
* googletrans 1.0
* pass 1.2

v0.18.8 (2023-01-07)

[albert]
* Give sensible defaults for usage history
* Fix memory weight not being loaded
* Merge frontend tab into general
* Support foot terminal
* Check for other instances _before_ laoding plugins

[plugins]
* Avoid starting indexing on file index serialization

[python]
* googletrans 1.0
* pass 1.2

v0.18.7 (2023-01-05)

[albert]
* Drop albertctl. Back to `albert <command>`

v0.18.6 (2023-01-05)

[albert]
* sendTrayNotification(…) add time parameter
* Support wezterm.

[plugins]
* Python 1.5
 * sendTrayNotification(…) add ms parameter
* Hash 1.5
 * Global query handler
 * Add copy 8 char action

[python]
* Pomodoro 1.1
* CopyQ 1.2

v0.18.5 (2023-01-04)

[albert]
* Support Kitty terminal
* Support Alacritty terminal
[plugins]
* [wbm] Show synopsis in tooltip

v0.18.4 (2023-01-03)

[albert]
* fix single instance mechanism

[python]
* Archive docker, curious segfaults
* Port 0.5 aur
* Port 0.5 awiki

v0.18.3 (2023-01-02)

[plugins]
* [wbm] Fix theme dir paths

v0.18.2 (2023-01-02)

[albert]
* Better diagnostics on frontend loading

v0.18.1 (2023-01-01)

[albert]
* Fix armhf builds

v0.18.0 (2022-12-31)

Note that there have been some breaking changes. The new plugin id format changed settings keys and config/cache/data paths. If you want to keep your old plugin settings you have to adjust the section names in the config file and adjust the paths in your config/cache/data dirs. (e.g. from `org.albert.files` to `files`). I'd recommend to start from scratch though, since too much changed.


[albert]

* Shorter plugin ids.
* Customizeable triggers (if the extension permits)
* Central plugin management
* More useful plugin metadata
* User customizable scoring parameters
  * Add user option memory decay
  * Add user option memory weight
* Finally scoring for _all_ items
* Inputline history goes to a file now
* Settingswidget overhaul
* Hello Qt6, C++20 👋
* Entirely new interface (see header files)
* Value typed Action class based on std::function
* Drop all former *Action classes
 * Free functions replace and extend action subclass functionality
* Updates to Item interface
* New and extended query handling interface classes
* Extended frontend interface
* New abstract plugin provider interface
  * Common plugin metadata
  * Maintainership is a thing now!
* Add StandardItem factory for better type deduction and readability
* Add bgexecutor class
* Add timeprinter
* Leaner logging
* Query design change (realtime, global, indexed)
* Add extension watcher template class
* Move XDG into the lib.


[plugins]

python 1.4 (0.18)
* Use system pybind
* 0.5 interface
* auto pip dependencies

files 1.2 (0.18)
* Drop bashlike completions. We have items.
* Settings per root path
* Add name filter dialog
* Add option watch filesystem
* Add option max depth

snippets 1.1 (0.18)
* files instead database

widgetsboxmodel 1.2 (0.18)
* Fading busy indicating settingsbutton
* Drop rich text
* Proper async query without flicker using statemachines
* Add input hint
* Add option show fallbacks on empty query
* Add option history search

Also new or ported to 0.18
* calculator 1.3 (0.18)
* system 1.2 (0.18)
* applications_xdg 1.3 (0.18)
* applications_macos 1.0 (0.18)
* ssh 1.3 (0.18)
* terminal 1.2 (0.18)
* chromium 1.2 (0.18)
* websearch 1.2 (0.18)
* urlhander 1.0 (0.18)
* hash 1.4 (0.18)
* template 0.0 (0.18)
* debug 1.1 (0.18)

Archived

* firefox
* qml box model
* mpris
* vbox

v0.17.6 (2022-10-08)

* Let users choose the chromium bookmarks path
* Fix https://github.com/albertlauncher/albert/issues/978

v0.17.5 (2022-10-05)

* Fix #1064.

v0.17.4 (2022-10-04)

* Fix https://github.com/albertlauncher/albert/issues/1117

v0.17.3 (2022-07-05)

[albert]
* Sloppy hotfix #1088. 0.18 will change DB entirely anyway.

v0.17.2 (2020-12-24)

[albert]
* Drop telemetry

[plugins]
* [wbm] Fix completion

v0.17.1 (2020-12-21)

[albert]
* Fix OBS builds
* Several fixes
* Archive virtualbox python extension

Copyright (c) 2018 Copyright Holder All Rights Reserved.v0.17.0 (2020-12-17)

[albert]
* Again break init order of Item for the sake of less boilerplate. Presumed this frequency indexStrings > actions > completion > urgency.
* Let shells handle splitting/quoting
* Add core as QueryHandler. Add restart, quit, settings action. Also to tray and cli.
* Drop shutil:: and let shells handle lexing

[plugins]
* FINALLY ARCHIVE EXTERNAL EXTENSIONS.
* New extension state : MissingDependencies
* Disable settings items of exts in this new state
* Use pybind v2.6.1
* [term] v1.1 Let shells handle lexing
* [calc] Add muparserInt option for hex calculations
* Use QLoggingCategory in all extensions
* Implicit dependency check for executables and Python modules
* [Pyv1.3] Adopt core changes. PyAPIv0.4. Changes to the API:
  * embedded module is called 'albert' now
  * Reflect core api changes:
    * Positional arguments of the standard item changes
    * New semantics of the term action constructors
      * String commandline will be executed in a shell
      * StringList commandline will be executed without shell
  * Add core version of iconLookup(StringList)
  * New metadata labels:
    * __version__: new versioning scheme iid_maj.iid_min.ext_version
    * __title__: former __prettyname__
    * __authors__: string or list
    * __exec_deps__: string or list
    * __py_deps__: string or list
    * __triggers__: string or list
  * Allow multiple triggers
  * Allow multiple authors

[python]
* Adpot APIv0.4 changes
* [locate] ' for basename '' for full path lookups
* [timer] Make notification stay.
* [baseconv] Python-style base prefixes to detect source base
* [texdoc] Add texdoc plugin
* [aur] add yay helper

Check the GitHub repositories for details.
https://github.com/albertlauncher/albert/commits/v0.17.0

v0.16.4 (2020-12-10)

Hotfix for #959

[albert]
* Fix tab order. Closes #866
* Update stale.yml

[plugins]
* [chromium] Chromium v1.1

[python]
* [docker] New extension prototype
* [timer] Use dbus instead of notify-send
* [units] v1.2 including to time conversion

v0.16.3 (2020-12-03)

* Hotfix for #955
* Archive defunct CoinMarketCap and Bitfinex extensions

v0.16.2 (2020-11-26)

[albert]
* Allow multiple instances of albert on different X sessions
* Fix super key not registering
* Add terms: Elementary, Tilix, QTerminal, Termite
* Fix build on FreeBSD
* Dont show fallbacks on triggered queries

[plugins]
* [Applications] Index desktop files on desktop
* [firefox] Rework v2
* [ssh] Respect the Include keyword
* [ssh] Allow hyphens to be part of hostnames
* [chromium] Add brave-browser to list of chromium based browsers.

[python]
* Add an offline emoji picker
* Add bitwarden extension
* Add xkcd plugin as submodule
* Add new extension: node.js evaluator
* Add new extension: php evaluator

v0.16.1 (2018-12-31)

* [albert] Fix default plugin lookup path
* [albert] Fix flicker when changing frontends
* [albert] Fix "Terminal option resets after a restart"
* [albert] Link libglobalshortcut statically
* [albert] Add a build flag for QtCharts
* [albert] Drop debug options if favor of QLoggingCategory env vars
* [plugins:ssh] Fix input regex. Sort by length then lexically.
* [plugins:ssh] Use backward compatible ssh url syntax
* [plugins:qml] Consistent form layout
* [modules:aur] Sort items by length first

v0.16.0 (2018-12-28)

* Add jekyll website as submodule
* New project structure (minimal shared lib)
* Let travis build against Ubuntu 18.04 and 16.04
* Backward compatibility for Ubuntu 16.04
* Let fuzzy require an additional character. Tolerance: floor((wordlen - 1)/3))
* Print logging category to stdout QT_LOGGING_RULES="*debug=false;driver.usb.debug=true"
* [Term] Change terminal action order: Let "Run w/o term" be the last one
* [VBox] Set default build switch for VirtualBox to OFF
* [Files] Add fancy icons to mime dialog
* [Py] Use ast to read metadata without loading the modules
* [Py] Additional constraint: Metadata have to be string literals (for ast)
* [Py] Additional constraint: Name modules according PEP8
* [Py/WinSwitch] Add close win action
* [Py/VBox] Add VirtualBox extension

v0.15.0 (2018-12-16)

* Usage graph in the settings (QtCharts (>=5.6))
* [QML] Frontend plugin requires ()5.9
* [QML] History search of the input now allows substring matching (Type and navigate)
* [QML] Store user input of every session
* New Python extension: Fortune
* New Python extension: Window switcher

v0.14.22 (2018-09-21)

* Telemetry is now opt-in.
* New Python extension: Pidgin
* New themes

v0.14.21 (2018-06-08)

* Bugfix release

v0.14.20 (2018-06-04)

* Bugfix release

v0.14.19 (2018-05-15)

* New Python extension: Datetime. (Time display and conversion. Supersedes the external extension)
* New Python extension: Bitfinex. (Quickly access Bitfinex markets)
* The file browse mode finally mimics bash completion behavior.

v0.14.18 (2018-03-23)

* Hotfix release

v0.14.17 (2018-03-23)

* New Python extension: Arch Wiki
* New option in applications extension: Use keywords for lookup
* New option in applications extension: Use generic name for lookup
* The _kvstore_ extension was renamed to _Snippets_ and got an improved config UI.

v0.14.16 (2018-03-09)

* New Python extension: Gnome dictionary (nikhilwanpal)
* New Python extension: Mathematica (Asger Hautop Drewsen)
* New Python extension: TeX to unicode (Asger Hautop Drewsen)
* New Python extension: IP address (Benedict Dudel)
* New Python extension: Multi Translate (David Britt)
* New Python extension: Emoji lookup (David Britt)
* New Python extension: Kaomoji lookup (David Britt)
* New Python extension: Timer
* New Python extension: Binance

v0.14.15 (2018-01-26)

* New Python extensions: CoinMarketCap, Trash, Pomodoro, Epoch, Packagist.
* New Python API PythonInterface/v0.2 (`configLocation()`, `dataLocation()`,`cacheLocation()`).

v0.14.14 (2017-12-06)

* New Python extension: npm (Benedict Dudel)

v0.14.13 (2017-11-25)

* Rich text support
* New AUR Python extension
* New `scrot` Python extension

v0.14.12 (2017-11-23)

* New CopyQ Python extension (Ported from external extension)

v0.14.11 (2017-11-19)

* New `locate` Python extension

v0.14.10 (2017-11-16)

* Bugfixes

v0.14.9 (2017-11-16)

* Better HiDPI support
* New commandline option for debug output (-d)

v0.14.8 (2017-11-14)

* New Gnote Python extension (Ported from external extension)
* New Tomboy Python extension (Ported from external extension)
* New Pacman Python extension
* New Pass Python extension
* New Kill Python extension

v0.14.7 (2017-11-03)

* Bugfixes

v0.14.6 (2017-10-31)

* New Wikipedia Python extension

v0.14.5 (2017-10-30)

* Bugfixes

v0.14.4 (2017-10-25)

* New base converter Python extension

v0.14.3 (2017-10-21)

* New Google Translate Python extension

v0.14.2 (2017-10-20)

* Bugfixes

v0.14.1 (2017-10-19)

* Bugfixes

v0.14.0 (2017-10-18)

* New Python Embedding extension
* New Python Eval Python extension
* New Debugging Python extension
* New Zeal Python extension
* New GoldenDict Python extension
* New Unit Converter Python extension (Gnu units)
* New Currency Converter Python extension (Google finance)

v0.13.1 (2017-00-30)

* Bugfixes

v0.13.0 (2017-09-28)

* Modular frontends
* QML frontend
* Tree structure in file index and "smart" indexing
* New KeyValue extension
* New Hash Generator extension
* Shell like completion for terminal extension

v0.12.0 (2017-06-09)

* Git-like ignore files
* Dedicated dialog for websearch editing.

v0.11.3 (2017-05-28)

*  Bugfixes

v0.11.2 (2017-05-13)

* <kbd>Home</kbd> and <kbd>End</kbd> now work for the results list holding <kbd>Ctrl</kbd>.

v0.11.1 (2017-04-16)

* Bugfixes

v0.11.0 (2017-04-16)

* Fine-grained control of the MIME types to be indexed.
* Extensions can now have multiple triggers.
* The sorting algorithm is now stable.
* Browse mode now lists the results in lexicographical order with directories before the files.
* The use of fallbacks has been disabled for triggered queries.
* Further the websearch extension now contains an URL handler.
* The qss property `selection-color` works as expected now.

v0.10.4 (2017-04-14)

* Bugfixes

v0.10.3 (2017-04-02)

* Terminal extension does no more show suggestions.
* Any shell querying dropped.
* Bugfixes

v0.10.2 (2017-03-24)

* Bugfixes

v0.10.1 (2017-03-20)

* Bugfixes

v0.10.0 (2017-03-19)

* Tab completion.
* Hovering over the input box the mouse wheel can now be used to browse the history.
* Spotlight themes (Bright, Dark and Space).
* New native extension: MPRIS
* New native extension: Secure Shell
* The terminal extension now provides the shell aliases too.
* File browse mode.
* The application extension allows to ignore the `OnlyShowIn`/`NotShowIn` keys.
* The bash script to clone the template extension is now deprecated and replaced by a Python script.

v0.9.5 (2017-03-13)

* Bugfixes

v0.9.4 (2017-03-01)

* Bugfixes

v0.9.3 (2017-02-05)

* Bugfixes

v0.9.2 (2017-01-30)

* Bugfixes

v0.9.1 (2017-01-23)

* Chooseable terminal command of a list of installed terminals.
* The app icon now uses a theme icon
* _Open terminal here_ action.
* Mostly fixes

v0.9.0 (2017-01-17)

* New extension system architecture
* New native extension: External extensions
* New native extension: Firefox
* Improved VirtualBox extension

v0.8.11 (2016-09-29)

* Plugin abstraction architecture
* New plugin support: Native C++ QPlugins
* New plugin support: Executables
* Single click activation of items
* Action modifiers
* Multithreading
* Core is now responsible for usage counting (obsoletes most serialization)
* Asynchronous query "live" results
* Tray icon
* New option: Show tray icon
* New extension: Debug

v0.8.10 (2016-06-23)

* New extension: Virtual Box
* Basic IPC
* Graceful termination on SIGHUP

v0.8.9 (2016-05-12)

* New option: Hide on close
* New option: Display shadow
* Graceful termination on SIGINT

v0.8.8 (2016-04-28)

* Single instance

v0.8.7.3 (2016-04-27), v0.8.7.2 (2016-04-03), v0.8.7.1 (2016-03-31)

* Hotfixes

v0.8.7 (2016-03-31)

* New extension: Terminal
* Project structure: Use libraries

v0.8.6 (2016-03-28)

* Restructured settings widget

v0.8.5 (2016-03-25)

* Custom icon lookup
* Make main window movable
* Show alternative actions o Tab
* New option: Always on top
* New option: Hide on focus out
* New option: Display icons
* New option: Display scrollbar
* Dozens of new themes: Dark, Bright, SolarizedDark, SolarizedBright in several colors.

v0.8.4 (2016-03-15)

* Show message box after ungraceful termination

v0.8.3 (2016-03-13)

* Restructured settings widget

v0.8.2 (2016-03-09)

* New option: Group separators for calculator
* New themes: Arc
* New theme: Numix

v0.8.1 (2016-03-04)

* Minor tasks and improvements

v0.8.0 (2015-10-27)

* New extension: System control

v0.7.7 (2015-10-16)

* Bring settings window to front if it is already open.

v0.7.6 (2015-10-15)

* Reorderable websearches
* Allow exclusive queries by trigger

v0.7.5 (2015-10-12)

* Graceful termination on SIGINT

v0.7.1 (2015-10-06), v0.7.2 (2015-10-07), v0.7.3 (2015-10-07), v0.7.4 (2015-10-08)

* Tasks, Hotfixes, minor changes

v0.7.0 (2015-10-05)

* Implement plugin architecture
* Port the modules
* Ignore file (".albertignore")
* Actions
* Threaded background indexing
* New themes

v0.6.0 (2014-11-12)

* Make action modifications configurable
* Command history

v0.5.0 (2014-11-06)

* Add configuration widget to configure the modules
* Make user interface themable
* Provide proof-of-concept themes
* Make actions modifiable
* Show action modifications in the list
* Use CMake build system

v0.4.0 (2014-10-16)

* Add settings widget
* Implement indexing and search algorithms 'prefixmatch' and 'fuzzy'

v0.3.0 (2014-09-12)

* Implement serialization of indexes
* New module: Applications
* New module: Bookmarks
* New module: Calculator
* New module: Web search

v0.2.0 (2014-09-08)

* Abstract module architecture
* New module: Files

v0.1.0 (2014-09-01)

* Basic user interface
* Hotkeymanager
