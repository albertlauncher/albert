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
