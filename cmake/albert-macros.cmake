# - Albert cmake macros
#
# Use albert_plugin() to add a plugin.
#
# albert_plugin(
#      [SOURCES ...]
#      [I18N_SOURCES ...]
#      [INCLUDE ...]
#      [LINK ...]
#      [QT ...]
# )
#
# Create a plugin target with the given name.
#
# Expects a metadata.json file in the source directory. Supported metadata keys are:
#
# |            Parameter |     Type     | Notes                                                                     |
# |---------------------:|:------------:|---------------------------------------------------------------------------|
# |                   id |   Reserved   | PROJECT_NAME added by CMake.                                              |
# |              version |   Reserved   | PROJECT_VERSION added by CMake.                                           |
# |                 name | local string | Human readable name.                                                      |
# |          description | local string | Brief, imperative description, e.g. "Open files".                         |
# |              license |    string    | SPDX license identifier. E.g. BSD-2-Clause, MIT, LGPL-3.0-only, …         |
# |                  url |    string    | Browsable online source code, issues etc.                                 |
# |              authors | string list  | List of copyright holders. Preferably using mentionable GitHub usernames. |
# | runtime_dependencies | string list  | Default: `[]`. Required libraries.                                        |
# |  binary_dependencies | string list  | Default: `[]`. Required executables.                                      |
# |  plugin_dependencies | string list  | Default: `[]`. Required plugins.                                          |
# |              credits | string list  | Default: `[]`. Attributions, mentions, third party library licenses, …    |
# |             loadtype |    string    | Default: `user`. `frontend` or `user`.                                    |
#
# Note: Local string types can be used to localize the metadata. (e.g. "name[de]": "Anwendungen")
#
# Translations files in a directory named 'i18n' are added automatically.
# The filenames must have the pattern <plugin_id>_<language_code>.ts.
# <plugin_id>.ts is the native plurals file.
#
# SOURCES
#     List source files.
#     Supports (nonrecursive) globbing patterns.
#     If unspecified the default is a recursive globbing pattern for:
#     include/*.h, src/*.h, src/*.cpp, src/*.mm, src/*.ui and <plugin_id>.qrc
#
# I18N_SOURCES
#     List translation source files.
#     Supports (nonrecursive) globbing patterns.
#     Use if some of the source files are disabled on certain platforms.
#     If unspecified defaults to the target sources.
#
# INCLUDE
#     List directories to include.
#     Shorthand for CMake target_include_directories(plugin_target …
#
# LINK
#     List of libraries to link.
#     Shorthand for CMake target_link_libraries(plugin_target …
#
# QT
#     List of Qt components to link.
#     Finds and links the given Qt components.
#

cmake_minimum_required(VERSION 3.19)  # string(JSON…

macro(albert_plugin_link_qt)
    set(options REQUIRED)
    set(one_value_args VERSION)
    set(multi_value_args QT)
    cmake_parse_arguments(ARG_QT "${options}" "${one_value_args}" "${multi_value_args}" ${ARGV})

    set(_qt_find_args COMPONENTS ${ARG_QT_UNPARSED_ARGUMENTS})
    if(ARG_QT_REQUIRED)
        list(APPEND _qt_find_args REQUIRED)
    endif()
    if(ARG_QT_VERSION)
        list(PREPEND _qt_find_args ${ARG_QT_VERSION})
    endif()

    find_package(Qt6 ${_qt_find_args})

    foreach(MODULE IN LISTS ARG_QT_UNPARSED_ARGUMENTS)
        target_link_libraries(${PROJECT_NAME} PRIVATE "Qt6::${MODULE}")
    endforeach()
endmacro()

#
# albert_plugin_add_dbus_interface(<xml_interface_spec>)
#
# Adds a DBus interface to the plugin from an XML interface specification.
#
# The the DBUS_SRCS are generated in the PROJECT_BINARY_DIR.
#
macro(albert_plugin_dbus_interface)
    set(options)
    set(one_value_args XML INCLUDE)
    set(multi_value_args)
    cmake_parse_arguments(ARG_DBUS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGV})

    get_filename_component(ARG_DBUS_XML_BASENAME ${ARG_DBUS_XML} NAME_WE)

    set_source_files_properties(${ARG_DBUS_XML} PROPERTIES NO_NAMESPACE ON)

    if(ARG_DBUS_INCLUDE)
        set_source_files_properties(${ARG_DBUS_XML} PROPERTIES INCLUDE ${ARG_DBUS_INCLUDE})
    endif()

    qt_add_dbus_interface(DBUS_SRCS ${ARG_DBUS_XML} ${ARG_DBUS_XML_BASENAME})

    target_sources(${PROJECT_NAME} PRIVATE
        ${ARG_DBUS_XML}
        ${DBUS_SRCS}
    )
endmacro()

macro(albert_plugin_sources)
    file(GLOB SOURCES ${ARGV})
    target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
endmacro()

macro(albert_plugin_link)
    target_link_libraries(${PROJECT_NAME} ${ARGV})
endmacro()

macro(albert_plugin_include_directories)
    target_include_directories(${PROJECT_NAME} ${ARGV})
endmacro()

macro(albert_plugin_compile_options)
    target_compile_options(${PROJECT_NAME} ${ARGV})
endmacro()

macro(albert_plugin_i18n)
    find_package(Qt6 6.0 REQUIRED COMPONENTS
        Core  # required by LinguistTools
        LinguistTools
    )

    # TODO ubuntu 26.04
    # qt_add_translations improves greatly with 6.7/6.8
    # for now plural files are full translation files with empty singulars
    # sure one could build custom targets and such but since this already
    # implemented in recent qt versions it's not worth the effort

    file(GLOB TS_FILES "i18n/${PROJECT_NAME}*.ts")

    if (NOT ARG_I18N_SOURCES)
        get_target_property(ARG_I18N_SOURCES ${PROJECT_NAME} SOURCES)
    else()
        file(GLOB ARG_I18N_SOURCES ${ARG_I18N_SOURCES})
    endif()

    if (TS_FILES)
        qt_add_translations(
            ${PROJECT_NAME}
            TS_FILES ${TS_FILES}
            SOURCES ${ARG_I18N_SOURCES}
            LUPDATE_OPTIONS
              # -no-obsolete
              -locations none
            # QM_FILES_OUTPUT_VARIABLE QM_FILES
        )
    endif()

    # install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/i18n")

    # Prepare a list of translations for the metadata
    foreach(TS_FILE ${TS_FILES})
        get_filename_component(BASENAME ${TS_FILE} NAME_WLE)

        if (NOT ${BASENAME} STREQUAL ${PROJECT_NAME})

            execute_process(
                COMMAND xmllint --xpath "count(//translation[not(@type='unfinished')])" ${TS_FILE}
                OUTPUT_VARIABLE FINISHED_COUNT
                COMMAND_ERROR_IS_FATAL ANY
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            execute_process(
                COMMAND xmllint --xpath "count(//translation)" ${TS_FILE}
                OUTPUT_VARIABLE TOTAL_COUNT
                COMMAND_ERROR_IS_FATAL ANY
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            string(REPLACE "${PROJECT_NAME}_" "" LANGUAGE_CODE ${BASENAME})

            list(APPEND TRANSLATIONS "${LANGUAGE_CODE} (${FINISHED_COUNT}/${TOTAL_COUNT})")

        endif()
    endforeach()
endmacro()

macro(albert_plugin_generate_metadata)
    if (NOT DEFINED PROJECT_VERSION)
        message(FATAL_ERROR "Plugin version is undefined")
    endif()

    file(READ "${PROJECT_SOURCE_DIR}/metadata.json" MD)

    string(JSON MD SET ${MD} "id" "\"${PROJECT_NAME}\"")

    string(JSON MD SET ${MD} "version" "\"${PROJECT_VERSION}\"")

    if (TRANSLATIONS)
        list(JOIN TRANSLATIONS "\", \"" TRANSLATIONS_CSV)
        string(JSON MD SET ${MD} "translations" "[\"${TRANSLATIONS_CSV}\"]")
    endif()

    # get_target_property(LIB_DEPENDENCIES ${PROJECT_NAME} LINK_LIBRARIES)
    # if (DEFINED LIB_DEPENDENCIES)
    #     list(JOIN LIB_DEPENDENCIES "\", \"" LIB_DEPENDENCIES_CSV)
    #     string(JSON MD SET ${MD} "lib_deps" "[\"${LIB_DEPENDENCIES_CSV}\"]")
    # endif()

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" "${MD}")

    target_sources(${PROJECT_NAME} PRIVATE
        "${PROJECT_SOURCE_DIR}/metadata.json"
        "${PROJECT_BINARY_DIR}/metadata.json"
    )
endmacro()

# This macro creates a plugin target with the given name.
macro(albert_plugin)
    set(arg_bool )
    set(arg_vals )
    set(arg_list SOURCES I18N_SOURCES INCLUDE LINK QT)
    cmake_parse_arguments(ARG "${arg_bool}" "${arg_vals}" "${arg_list}" ${ARGV})

    if (NOT DEFINED ARG_SOURCES)
        file(GLOB_RECURSE ARG_SOURCES
            src/*.h
            src/*.hpp
            src/*.cpp
            src/*.mm
            src/*.ui
            include/*.h
            ${PROJECT_NAME}.qrc
        )
    else()
        file(GLOB ARG_SOURCES ${ARG_SOURCES})
    endif()

    add_library(${PROJECT_NAME} SHARED ${ARG_SOURCES})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        PREFIX ""  # no libfoo
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_CAST_FROM_ASCII)

    # Append. There are defaults.
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY AUTOMOC_MACRO_NAMES "ALBERT_PLUGIN")

    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

    albert_plugin_sources(README.md)  # if readme exists add to sources

    if (DEFINED ARG_INCLUDE)
        albert_plugin_include_directories(${ARG_INCLUDE})
    endif()

    if (DEFINED ARG_LINK)
        albert_plugin_link(${ARG_LINK})
    endif()
    albert_plugin_link(PRIVATE albert::libalbert)

    if (DEFINED ARG_QT)
        albert_plugin_link_qt(${ARG_QT})
    endif()

    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    )

    # after add_target, uses SOURCES
    # before metadata, defines TRANSLATIONS
    albert_plugin_i18n()
    albert_plugin_generate_metadata()
endmacro()
