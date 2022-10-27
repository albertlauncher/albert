#include("${CMAKE_CURRENT_LIST_DIR}/AlbertTargets.cmake")
#include("${CMAKE_BINARY_DIR}/AlbertTargets.cmake")

#message("{CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}")
#message("{CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}")
#
#get_filename_component(albert_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
#include("${albert_CMAKE_DIR}/AlbertTargets.cmake")

####################### nifty stuff for library developers

set(md_bool USER)
set(md_vals ID VERSION NAME DESCRIPTION LICENSE URL)
set(md_list MAINTAINERS QT_DEPENDENCIES LIB_DEPENDENCIES EXEC_DEPENDENCIES)

function(albert_plugin_generate_metadata_json)
    cmake_parse_arguments(MD "${md_bool}" "${md_vals}" "${md_list}" ${ARGN})

    if (NOT DEFINED MD_ID)
        message(FATAL_ERROR "${PROJECT_NAME}: Plugin identifier is undefined")
    endif()
    if (NOT DEFINED MD_VERSION)
        message(FATAL_ERROR "Plugin version is undefined")
    endif()
    if (NOT DEFINED MD_NAME)
        message(FATAL_ERROR "Plugin name is undefined")
    endif()
    if (NOT DEFINED MD_DESCRIPTION)
        message(FATAL_ERROR "Plugin description is undefined")
    endif()
    if (NOT DEFINED MD_LICENSE)
        message(FATAL_ERROR "Plugin license is undefined")
    endif()
    if (NOT DEFINED MD_URL)
        message(FATAL_ERROR "Plugin url is undefined")
    endif()

    list(APPEND MD "\"id\": \"${MD_ID}\"")
    list(APPEND MD "\"version\": \"${MD_VERSION}\"")
    list(APPEND MD "\"name\": \"${MD_NAME}\"")
    list(APPEND MD "\"description\": \"${MD_DESCRIPTION}\"")
    list(APPEND MD "\"license\": \"${MD_LICENSE}\"")
    list(APPEND MD "\"url\": \"${MD_URL}\"")

    if (MD_USER)
        list(APPEND MD "\"user\": \"${MD_USER}\"")
    endif()

    if (DEFINED MD_MAINTAINERS)
        list(JOIN MD_MAINTAINERS "\", \"" X)
        list(APPEND MD "\"maintainers\": [\"${X}\"]")
    endif()
    if (DEFINED MD_QT_DEPENDENCIES)
        list(JOIN MD_QT_DEPENDENCIES "\", \"" X)
        list(APPEND MD "\"qt_deps\": [\"${X}\"]")
    endif()
    if (DEFINED MD_LIB_DEPENDENCIES)
        list(JOIN MD_LIB_DEPENDENCIES "\", \"" X)
        list(APPEND MD "\"lib_deps\": [\"${X}\"]")
    endif()
    if (DEFINED MD_EXEC_DEPENDENCIES)
        list(JOIN MD_EXEC_DEPENDENCIES "\", \"" X)
        list(APPEND MD "\"exec_deps\": [\"${X}\"]")
    endif()

    # Build authors list from git
    execute_process(COMMAND bash -c "git -C ${CMAKE_CURRENT_SOURCE_DIR} log --pretty=format:%an .|sort|uniq"
            OUTPUT_VARIABLE AUTHORS OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "\n" ";" AUTHORS ${AUTHORS})

    if (DEFINED MD_AUTHORS)
        list(JOIN MD_AUTHORS "\", \"" X)
        list(APPEND MD "\"authors\": [\"${X}\"]")
    endif()

    list(JOIN MD ", " MD)

    # Create the metadata in the build dir
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" "{${MD}}")
    #message("${CMAKE_CURRENT_BINARY_DIR}/metadata.json {${MD}}")
endfunction()

function(albert_plugin_add_default_target)
    # Add a target
    file(GLOB_RECURSE ../src src/*.cpp src/*.h src/*.mm *.qrc *.ui)
    add_library(${PROJECT_NAME} SHARED ${SRC})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE albert::albert)

    # Add the metadata as a macro
    #    target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
    #    target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_METADATA='${METADATA}')
endfunction()

function(albert_plugin_generate_export_header)
    generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "export.h")
endfunction()

function(albert_plugin_add_default_target_properties)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN 1
            INSTALL_RPATH "$ORIGIN"
            )
endfunction()

function(albert_plugin_add_default_install)
    install(
            TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins
    )
endfunction()

function(albert_plugin_add_qt_dependencies)
    foreach(arg IN LISTS ARGN)
        find_package(Qt6 REQUIRED COMPONENTS ${arg})
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::${arg})
    endforeach()
endfunction()

function(albert_plugin_default)
    albert_plugin_add_default_target()
    albert_plugin_add_default_target_properties()
    albert_plugin_add_default_install()
    albert_plugin_add_qt_dependencies(${ARGV})
endfunction()

function(albert_plugin)
    cmake_parse_arguments(MD "${md_bool}" "${md_vals}" "${md_list}" ${ARGV})

    project(${MD_ID} VERSION ${MD_VERSION})

    albert_plugin_add_default_target()
    albert_plugin_generate_metadata_json(${ARGV})
    albert_plugin_generate_export_header()
    albert_plugin_add_default_target_properties()
    albert_plugin_add_qt_dependencies(${MD_QT_DEPENDENCIES})
    albert_plugin_add_default_install()

endfunction()
