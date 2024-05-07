# - Albert cmake macros
#
# Use albert_plugin() to add a plugin.
#
#     albert_plugin(
#          SOURCE_FILES
#          [INCLUDE_DIRECTORIES ...]
#          [LINK_LIBRARIES ...]
#          [METADATA filepath]
#          [TS_FILES ts_files...]
#     )
#
#     Creates a plugin target with the given name.
#
#     SOURCE_FILES
#         List of target source files. Supports globbing patterns.
#         The METADATA file is automatically added to the sources.
#
#     INCLUDE_DIRECTORIES
#         List of include directories.
#         Shorthand for CMake target_include_directories(plugin_target …
#
#     LINK_LIBRARIES
#         List of link libraries.
#         Shorthand for CMake target_link_libraries(plugin_target …
#
#     METADATA
#         Path to the metadata.json file. Defaults to "metadata.json".
#
#     TS_FILES
#         Translation files. Should have the pattern <plugin_id>_<language_code>.ts .
#

cmake_minimum_required(VERSION 3.19)  # string(JSON…


# on macOS include the macports lookup path
if (APPLE)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /opt/local)
endif()


macro(albert_plugin_add_default_target)

    if (NOT DEFINED PROJECT_VERSION)
        message(FATAL_ERROR "Plugin version is undefined")
    endif()

    set(arg_bool )
    set(arg_vals METADATA)
    set(arg_list
        SOURCE_FILES
        INCLUDE_DIRECTORIES
        LINK_LIBRARIES
        TS_FILES
    )
    cmake_parse_arguments(ARG "${arg_bool}" "${arg_vals}" "${arg_list}" ${ARGV})

    if (NOT DEFINED ARG_SOURCE_FILES)
        message(FATAL_ERROR "No sources specified.")
    endif()

    if (NOT DEFINED ARG_METADATA)
        set(ARG_METADATA "metadata.json")
    endif()

    file(GLOB GLOBBED_SRC ${ARG_SOURCE_FILES})

    add_library(${PROJECT_NAME} SHARED ${GLOBBED_SRC} ${ARG_METADATA})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    target_compile_options(${PROJECT_NAME} PRIVATE ${ALBERT_COMPILE_OPTIONS})

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})  # metadata.json
    if (DEFINED ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${PROJECT_NAME} ${ARG_INCLUDE_DIRECTORIES})
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE albert::albert)
    if (DEFINED ARG_LINK_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} ${ARG_LINK_LIBRARIES})
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
    )

    if (DEFINED ARG_TS_FILES)
        get_target_property(SRCS ${PROJECT_NAME} SOURCES)
        qt_add_translations(${PROJECT_NAME}
            TS_FILES ${ARG_TS_FILES}
            SOURCES ${SRCS}
        )
    endif()

    #include(GenerateExportHeader)
    #generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "export.h")

    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    )

endmacro()


macro(albert_plugin_generate_metadata_json)

    # read metadata.json
    file(READ ${ARG_METADATA} MD)

    # add project metadata
    string(JSON MD SET ${MD} "id" "\"${PROJECT_NAME}\"")

    string(JSON MD SET ${MD} "version" "\"${PROJECT_VERSION}\"")

    get_target_property(LIB_DEPENDENCIES ${PROJECT_NAME} LINK_LIBRARIES)
    if (DEFINED LIB_DEPENDENCIES)
        list(JOIN LIB_DEPENDENCIES "\", \"" LIB_DEPENDENCIES_CSV)
        string(JSON MD SET ${MD} "lib_deps" "[\"${LIB_DEPENDENCIES_CSV}\"]")
    endif()

    # Create the metadata in the build dir
    message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/metadata.json ${MD}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" "${MD}")

endmacro()


macro(albert_plugin)

    albert_plugin_add_default_target(${ARGV})
    albert_plugin_generate_metadata_json()

endmacro()
