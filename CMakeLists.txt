# SPDX-FileCopyrightText: 2024 Manuel Schneider

cmake_minimum_required(VERSION 3.22)  # Ubuntu 22.04

# dont touch! set by metatool
set(PROJECT_VERSION 0.26.9)

project(albert
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Keyboard launcher"
    HOMEPAGE_URL "https://albertlauncher.github.io"
    LANGUAGES CXX
)
if(APPLE)
    enable_language(OBJCXX)  # used by pch
endif()
set(PROJECT_DISPLAY_NAME "Albert")

# Local cmake modules (also CMake uses this in a pretty obscure way, e.g. for the plist)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(ALBERT_COMPILE_OPTIONS
    #-Werror
    -Wall
    -Wextra
    -Wpedantic

    # warnings that should be errors
    -Werror=return-type

    # -Wconversion
    # -Weffc++
    -Winline
    -Wmissing-field-initializers
    -Wshadow
    -Wstrict-aliasing

    # Use this from time to time
    # -Weverything
    # -Wdocumentation
    # -Wno-c++98-compat
    # -Wno-c++20-compat
    # -Wno-c++98-compat-pedantic

    # ??
    # -Wl,-undefined
)

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


### Dependencies ##############################################################

# QNotification

add_subdirectory(lib/QNotification EXCLUDE_FROM_ALL)

# QHotkey

set(QT_DEFAULT_MAJOR_VERSION 6)
set(BUILD_SHARED_LIBS FALSE)
set(QHOTKEY_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/QHotkey EXCLUDE_FROM_ALL)
target_compile_options(qhotkey PRIVATE -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-parameter -Wno-shadow)


### Lib  ######################################################################

set(TARGET_LIB lib${PROJECT_NAME})

set(LIB_PUBLIC_HEADER
    ${PROJECT_BINARY_DIR}/include/albert/config.h  # generated
    ${PROJECT_BINARY_DIR}/include/albert/export.h  # generated

    include/albert/action.h
    include/albert/backgroundexecutor.h
    include/albert/extension.h
    include/albert/extensionplugin.h
    include/albert/extensionregistry.h
    include/albert/extensionwatcher.h
    include/albert/fallbackhandler.h
    include/albert/frontend.h
    include/albert/globalqueryhandler.h
    include/albert/iconprovider.h
    include/albert/indexitem.h
    include/albert/indexqueryhandler.h
    include/albert/inputhistory.h
    include/albert/item.h
    include/albert/logging.h
    include/albert/matchconfig.h
    include/albert/matcher.h
    include/albert/notification.h
    include/albert/plugindependency.h
    include/albert/plugininstance.h
    include/albert/pluginloader.h
    include/albert/pluginmetadata.h
    include/albert/pluginprovider.h
    include/albert/property.h
    include/albert/query.h
    include/albert/rankitem.h
    include/albert/standarditem.h
    include/albert/telemetryprovider.h
    include/albert/timeit.h
    include/albert/triggerqueryhandler.h
    include/albert/util.h
)

set(LIB_SRC
    src/app/app.cpp
    src/app/app.h
    src/app/appqueryhandler.cpp
    src/app/appqueryhandler.h
    src/app/messagehandler.cpp
    src/app/messagehandler.h
    src/app/pluginqueryhandler.cpp
    src/app/pluginqueryhandler.h
    src/app/qtpluginloader.cpp
    src/app/qtpluginloader.h
    src/app/qtpluginprovider.cpp
    src/app/qtpluginprovider.h
    src/app/report.cpp
    src/app/report.h
    src/app/rpcserver.cpp
    src/app/rpcserver.h
    src/app/telemetry.cpp
    src/app/telemetry.h
    src/app/telemetryprovider.cpp
    src/app/triggersqueryhandler.cpp
    src/app/triggersqueryhandler.h

    src/common/action.cpp
    src/common/extension.cpp
    src/common/item.cpp
    src/common/rankitem.cpp

    src/frontend/frontend.cpp
    src/frontend/inputhistory.cpp
    src/frontend/itemsmodel.cpp
    src/frontend/itemsmodel.h
    src/frontend/session.cpp
    src/frontend/session.h

    src/platform/platform.h
    src/platform/signalhandler.h

    src/plugin/extensionregistry.cpp
    src/plugin/plugin.cpp
    src/plugin/plugin.h
    src/plugin/plugininstance.cpp
    src/plugin/pluginloader.cpp
    src/plugin/pluginprovider.cpp
    src/plugin/pluginregistry.cpp
    src/plugin/pluginregistry.h
    src/plugin/topologicalsort.hpp

    src/query/fallbackhandler.cpp
    src/query/globalqueryhandler.cpp
    src/query/query.cpp
    src/query/queryengine.cpp
    src/query/queryengine.h
    src/query/queryexecution.cpp
    src/query/queryexecution.h
    src/query/triggerqueryhandler.cpp
    src/query/usagedatabase.cpp
    src/query/usagedatabase.h

    src/settings/pluginswidget/pluginsmodel.cpp
    src/settings/pluginswidget/pluginsmodel.h
    src/settings/pluginswidget/pluginssortproxymodel.cpp
    src/settings/pluginswidget/pluginssortproxymodel.h
    src/settings/pluginswidget/pluginswidget.cpp
    src/settings/pluginswidget/pluginswidget.h
    src/settings/pluginswidget/pluginwidget.cpp
    src/settings/pluginswidget/pluginwidget.h

    src/settings/querywidget/fallbacksmodel.cpp
    src/settings/querywidget/fallbacksmodel.h
    src/settings/querywidget/queryhandlermodel.cpp
    src/settings/querywidget/queryhandlermodel.h
    src/settings/querywidget/querywidget.cpp
    src/settings/querywidget/querywidget.h

    src/settings/settingswindow.cpp
    src/settings/settingswindow.h

    src/util/extensionplugin.cpp
    src/util/iconprovider.cpp
    src/util/indexitem.cpp
    src/util/indexqueryhandler.cpp
    src/util/itemindex.cpp
    src/util/itemindex.h
    src/util/levenshtein.cpp
    src/util/levenshtein.h
    src/util/matcher.cpp
    src/util/notification.cpp
    src/util/standarditem.cpp
    src/util/util.cpp

    src/config.h.in
)

if (WIN32)
    list(APPEND LIB_SRC
        src/platform/win/signalhandler.cpp
    )
elseif (APPLE)
    list(APPEND LIB_SRC
        src/platform/unix/signalhandler.cpp
        src/platform/mac/platform.mm
    )
elseif(UNIX)  # assume xdg
    list(APPEND LIB_SRC
        src/platform/unix/signalhandler.cpp
        src/platform/xdg/iconlookup.cpp
        src/platform/xdg/iconlookup.h
        src/platform/xdg/platform.cpp
        src/platform/xdg/themefileparser.cpp
        src/platform/xdg/themefileparser.h
    )
endif()

set(LIB_UI
    src/settings/querywidget/querywidget.ui
    src/settings/settingswindow.ui
)

add_library(${TARGET_LIB} SHARED
    ${LIB_PUBLIC_HEADER}
    ${LIB_SRC}
    ${LIB_UI}
    resources/resources.qrc
)

add_library("${PROJECT_NAME}::${TARGET_LIB}" ALIAS ${TARGET_LIB})

include(GNUInstallDirs)
include(GenerateExportHeader)

generate_export_header(${TARGET_LIB}
    BASE_NAME ${PROJECT_NAME}
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/albert/export.h"
)

target_precompile_headers(${TARGET_LIB} PRIVATE ${LIB_PUBLIC_HEADER})

target_compile_options(${TARGET_LIB} PRIVATE ${ALBERT_COMPILE_OPTIONS})

target_include_directories(${TARGET_LIB}
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${PROJECT_BINARY_DIR}/include/albert"
        "${PROJECT_SOURCE_DIR}/include/albert"
        src
        src/app
        src/common
        src/frontend
        src/handlers
        src/platform
        src/platform/mac
        src/platform/unix
        src/platform/xdg
        src/plugin
        src/query
        src/settings
        src/settings/pluginswidget
        src/settings/querywidget
        src/util
)

configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/include/albert/config.h"
    @ONLY
)

find_package(Qt6 6.2 REQUIRED COMPONENTS # Ubuntu 22.04
    Core
    Concurrent
    Network
    Sql
    Svg
    Widgets
    LinguistTools
)

target_link_libraries(${TARGET_LIB}
    PRIVATE
        QHotkey::QHotkey
        QNotifications::QNotifications
        Qt6::Concurrent
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
)

if(APPLE)
    target_link_libraries(${TARGET_LIB} PRIVATE objc "-framework Cocoa")
    target_compile_options(${TARGET_LIB} PRIVATE "-fobjc-arc")
elseif(UNIX)
    if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        target_compile_definitions(${TARGET_LIB}
            PRIVATE -DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}"
        )
    endif()
endif()

set_target_properties(${TARGET_LIB} PROPERTIES
    #INSTALL_RPATH "$ORIGIN"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_FRAMEWORK_IDENTIFIER "org.albertlauncher.albert.interface"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
    OUTPUT_NAME "${PROJECT_NAME}"
    PUBLIC_HEADER "${LIB_PUBLIC_HEADER}"
    SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"  # also mach-o compat version
    # NO_SONAME true # do _not_ add. linkers other that ldd will make troubles using non relative paths
    VERSION ${PROJECT_VERSION}
    VISIBILITY_INLINES_HIDDEN 1
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

### Internationalization

file(GLOB TS_FILES i18n/*.ts)

# Source files to be translated (separate because some files may be masked)
file(GLOB_RECURSE TRANSLATION_SOURCE_FILES
    include/.h
    src/*.h
    src/*.ui
    src/*.mm
    src/*.cpp
)

qt_add_translations(
    ${TARGET_LIB}
    TS_FILES ${TS_FILES}
    SOURCES ${TRANSLATION_SOURCE_FILES}
    LUPDATE_OPTIONS
      # -no-obsolete
      -locations none
)

# For convenience, QtCreator does not show the umbrella target
add_custom_target(global_lupdate DEPENDS update_translations)

### Export/Install

include(CMakePackageConfigHelpers)

set(LIB_EXPORT_NAME "${PROJECT_NAME}-targets")
set(LIB_TARGETS_FILE "${LIB_EXPORT_NAME}.cmake")
set(LIB_CONFIG_FILE "${PROJECT_NAME}-config.cmake")
set(LIB_VERSION_FILE "${PROJECT_NAME}-config-version.cmake")
set(LIB_MACROS_FILE "${PROJECT_NAME}-macros.cmake")
set(LIB_FIND_ALBERT_FILE "FindAlbert.cmake")
set(LIB_CMAKE_MODULE_DIR "${PROJECT_SOURCE_DIR}/cmake")
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Albert")

# Install the target
# https://cmake.org/cmake/help/latest/command/install.html#targets
install(
    TARGETS ${TARGET_LIB}
    EXPORT ${LIB_EXPORT_NAME}  # just an association
    FRAMEWORK DESTINATION .
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/albert
)

# Create a targets file in install tree
# https://cmake.org/cmake/help/latest/command/install.html#export
# By default the generated file will be called <export-name>.cmake
install(
    EXPORT ${LIB_EXPORT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Create a targets file in build tree (matching install(EXPORT))
# https://cmake.org/cmake/help/latest/command/export.html#exporting-targets-matching-install-export
# Seems like a shorthand for export(TARGETS)
export(
   EXPORT ${LIB_EXPORT_NAME}
   NAMESPACE ${PROJECT_NAME}::
   FILE "${PROJECT_BINARY_DIR}/${LIB_TARGETS_FILE}"
)

include(CMakePackageConfigHelpers)

# Create version file in build tree
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${LIB_VERSION_FILE}"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion
)

# Create config file in build tree
configure_package_config_file(
   "${LIB_CMAKE_MODULE_DIR}/${LIB_CONFIG_FILE}.in"
   "${PROJECT_BINARY_DIR}/${LIB_CONFIG_FILE}"
   INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Copy the macros to the build directory
configure_file(
    "${LIB_CMAKE_MODULE_DIR}/${LIB_MACROS_FILE}"
    "${PROJECT_BINARY_DIR}/${LIB_MACROS_FILE}"
    COPYONLY
)

# Copy the dummy FindAlbert to the build directory
configure_file(
    "${LIB_CMAKE_MODULE_DIR}/${LIB_FIND_ALBERT_FILE}.in"
    "${PROJECT_BINARY_DIR}/${LIB_FIND_ALBERT_FILE}"
    @ONLY
)

install(FILES
    "${PROJECT_BINARY_DIR}/${LIB_CONFIG_FILE}"
    "${PROJECT_BINARY_DIR}/${LIB_VERSION_FILE}"
    "${PROJECT_BINARY_DIR}/${LIB_MACROS_FILE}"
    DESTINATION ${INSTALL_CONFIGDIR}
)


### App  ######################################################################

set(TARGET_APP ${CMAKE_PROJECT_NAME})

add_executable(${TARGET_APP} MACOSX_BUNDLE "src/main.cpp")

if (APPLE)

    set(ICON_NAME "albert")
    set(ICON_PATH "dist/macos/${ICON_NAME}.icns")
    target_sources(${TARGET_APP} PRIVATE ${ICON_PATH})

    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(${TARGET_APP} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_DISPLAY_NAME}"  # "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2024 Manuel Schneider"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.albertlauncher.albert"
        MACOSX_BUNDLE_ICON_FILE "${ICON_NAME}"
        MACOSX_BUNDLE_INFO_STRING "Albert keyboard launcher"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    )

elseif (UNIX)

    install(FILES dist/xdg/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES resources/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

endif()

target_link_libraries(${TARGET_APP} PRIVATE ${TARGET_LIB})


    #INSTALL_RPATH_USE_LINK_PATH TRUE
    #INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    #BUILD_WITH_INSTALL_RPATH FALSE
    #INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
    #INSTALL_RPATH "$ORIGIN/../Frameworks/albert/"  # Set the RPATH for the library lookup

install(
    TARGETS ${TARGET_APP}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


### Plugins ###################################################################

# Since the cmake module path of this project contains a dummy FindAlbert.cmake
# CMake uses this one instead of searching systemwide.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}")

# # on macOS include the macports lookup path
# if (APPLE)
#     list(APPEND CMAKE_PREFIX_PATH "/opt/local")
#     message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
# endif()
#2024: Drop ports use brew only
# if (APPLE)
#     list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
#     message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
# endif()

option(BUILD_PLUGINS "Build plugins" ON)
set(PLUGINS_DIR "${CMAKE_CURRENT_LIST_DIR}/plugins")
if (BUILD_PLUGINS)
    file(GLOB ENTRIES RELATIVE "${PLUGINS_DIR}" plugins/*)
    list(FILTER ENTRIES EXCLUDE REGEX "^\\..+")
    foreach(ENTRY ${ENTRIES})
        if(IS_DIRECTORY "${PLUGINS_DIR}/${ENTRY}")
            string(TOUPPER ${ENTRY} UPPER)
            option(BUILD_PLUGIN_${UPPER} "Build plugin ${ENTRY}" ON)
            if (BUILD_PLUGIN_${UPPER})
                add_subdirectory("${PLUGINS_DIR}/${ENTRY}")
            endif()
        endif()
    endforeach()
endif()


### Test ######################################################################

option(BUILD_TESTS "Build tests (Requires QTest)" OFF)
if (BUILD_TESTS)
    include(CTest)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    get_target_property(SRC_TST ${TARGET_LIB} SOURCES)
    get_target_property(INC_TST ${TARGET_LIB} INCLUDE_DIRECTORIES)
    get_target_property(LIBS_TST ${TARGET_LIB} LINK_LIBRARIES)
    get_target_property(CXX_STD_TST ${TARGET_LIB} CXX_STANDARD)

    set(TARGET_TST ${CMAKE_PROJECT_NAME}_test)

    add_executable(${TARGET_TST} ${SRC_TST} test/test.h test/test.cpp)

    target_include_directories(${TARGET_TST} PRIVATE ${INC_TST} test)
    target_link_libraries(${TARGET_TST} PRIVATE ${LIBS_TST} Qt6::Test)
    set_target_properties(${TARGET_TST} PROPERTIES
        CXX_STANDARD ${CXX_STD_TST}
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    add_test(NAME ${TARGET_TST} COMMAND ${TARGET_TST})

endif()


### Packaging #################################################################


if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_CHECKSUM "SHA256")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_DISPLAY_NAME} executable")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_DISPLAY_NAME}-v${PROJECT_VERSION}")
    set(CPACK_PACKAGE_NAME "${PROJECT_DISPLAY_NAME}")
    #set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/${ICON_PATH}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "manuelschneid3r")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.md")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_MONOLITHIC_INSTALL TRUE)

    configure_file("cmake/bundle-macos.cmake.in" "${PROJECT_BINARY_DIR}/bundle-macos.cmake" @ONLY)
    install(SCRIPT "${PROJECT_BINARY_DIR}/bundle-macos.cmake")

    include(CPack)
endif()
