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
# Expects a metadata.json file in the source directory.
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

macro(_albert_plugin_add_target)

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

        if (NOT ARG_SOURCES)
            message(FATAL_ERROR "No source files.")
        endif()

    else()

        file(GLOB ARG_SOURCES ${ARG_SOURCES})

    endif()


    # Instruct CMake to run moc automatically when needed.

    add_library(${PROJECT_NAME} SHARED ${ARG_SOURCES})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN j
        PREFIX ""  # no libfoo
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY AUTOMOC_MACRO_NAMES "ALBERT_PLUGIN")

    target_compile_options(${PROJECT_NAME} PRIVATE ${ALBERT_COMPILE_OPTIONS})

    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})
    if (DEFINED ARG_INCLUDE)
        target_include_directories(${PROJECT_NAME} ${ARG_INCLUDE})
    endif()

    if (DEFINED ARG_LINK)
        target_link_libraries(${PROJECT_NAME} ${ARG_LINK})
    endif()

    if (DEFINED ARG_QT)
        find_package(Qt6 6.2 REQUIRED COMPONENTS ${ARG_QT})
        foreach(MODULE ${ARG_QT})
            target_link_libraries(${PROJECT_NAME} PRIVATE "Qt6::${MODULE}")
        endforeach()
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE albert::libalbert)

    #include(GenerateExportHeader)
    #generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "export.h")

    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    )

endmacro()


macro(_albert_plugin_add_translations)

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
            # LUPDATE_OPTIONS -no-obsolete
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


macro(_albert_plugin_generate_metadata_json)

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


macro(albert_plugin)

    set(arg_bool )
    set(arg_vals )
    set(arg_list SOURCES I18N_SOURCES INCLUDE LINK QT)
    cmake_parse_arguments(ARG "${arg_bool}" "${arg_vals}" "${arg_list}" ${ARGV})

    _albert_plugin_add_target()
    # after add_target, uses SOURCES
    # before metadata, defines TRANSLATIONS
    _albert_plugin_add_translations()
    _albert_plugin_generate_metadata_json()

endmacro()

