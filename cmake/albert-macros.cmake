# - Albert cmake macros
#
# Use albert_plugin() to add a plugin.
#
#     albert_plugin(
#          [SOURCE_FILES ...]
#          [INCLUDE_DIRECTORIES ...]
#          [LINK_LIBRARIES ...]
#     )
#
#     Create a plugin target with the given name.
#
#     Expects a metadata.json file in the source directory.
#
#     Translations files in a directory named 'i18n' are added automatically.
#     The filenames must have the pattern <plugin_id>_<language_code>.ts.
#     <plugin_id>.ts is the native plurals file.
#
#     SOURCE_FILES
#         List source files.
#         Supports (nonrecursive) globbing patterns. If unspecified the default
#         is a recursive globbing pattern for:
#         include/*.h, src/*.h, src/*.cpp, src/*.mm, src/*.ui and <plugin_id>.qrc
#
#     INCLUDE_DIRECTORIES
#         List of include directories.
#         Shorthand for CMake target_include_directories(plugin_target …
#
#     LINK_LIBRARIES
#         List of link libraries.
#         Shorthand for CMake target_link_libraries(plugin_target …
#

cmake_minimum_required(VERSION 3.19)  # string(JSON…


# on macOS include the macports lookup path
if (APPLE)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /opt/local)
endif()


macro(_albert_plugin_add_target)

    if (NOT DEFINED ARG_SOURCE_FILES)

        file(GLOB_RECURSE ARG_SOURCE_FILES
            src/*.h
            src/*.cpp
            src/*.mm
            src/*.ui
            include/*.h
            ${PROJECT_NAME}.qrc
        )

        if (NOT ARG_SOURCE_FILES)
            message(FATAL_ERROR "No source files.")
        endif()

    endif()


    add_library(${PROJECT_NAME} SHARED ${ARG_SOURCE_FILES} )
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
        PREFIX ""  # no liblib
    )

    target_compile_options(${PROJECT_NAME} PRIVATE ${ALBERT_COMPILE_OPTIONS})

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    if (DEFINED ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${PROJECT_NAME} ${ARG_INCLUDE_DIRECTORIES})
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE albert::albert)
    if (DEFINED ARG_LINK_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} ${ARG_LINK_LIBRARIES})
    endif()

    #include(GenerateExportHeader)
    #generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "export.h")

    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    )

endmacro()


macro(_albert_plugin_add_translations)

    # todo ubuntu 26.04 qt_add_translations improves greatly with 6.8
    file(GLOB TS_FILES "i18n/${PROJECT_NAME}*.ts")

    if (TS_FILES)
        qt_add_translations(
            ${PROJECT_NAME}
            TS_FILES ${TS_FILES}
            LUPDATE_OPTIONS -no-obsolete
            # QM_FILES_OUTPUT_VARIABLE QM_FILES
        )
    endif()

    # install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/i18n")

    # Prepare a list of translations for the metadata
    foreach(TS_FILE ${TS_FILES})
        get_filename_component(BASENAME ${TS_FILE} NAME_WLE)
        if (NOT ${BASENAME} STREQUAL ${PROJECT_NAME})
            string(REPLACE "${PROJECT_NAME}_" "" LANGUAGE_CODE ${BASENAME})
            list(APPEND TRANSLATIONS ${LANGUAGE_CODE})
        endif()
    endforeach()

endmacro()


macro(_albert_plugin_generate_metadata_json)

    if (NOT DEFINED PROJECT_VERSION)
        message(FATAL_ERROR "Plugin version is undefined")
    endif()

    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json" MD)

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
        "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
        "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    )

endmacro()


macro(albert_plugin)

    set(arg_bool )
    set(arg_vals )
    set(arg_list SOURCE_FILES INCLUDE_DIRECTORIES LINK_LIBRARIES)
    cmake_parse_arguments(ARG "${arg_bool}" "${arg_vals}" "${arg_list}" ${ARGV})

    _albert_plugin_add_target()
    _albert_plugin_add_translations()  # before metadata, defines TRANSLATIONS
    _albert_plugin_generate_metadata_json()

endmacro()

