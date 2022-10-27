#include("${CMAKE_CURRENT_LIST_DIR}/AlbertTargets.cmake")
#include("${CMAKE_BINARY_DIR}/AlbertTargets.cmake")

#message("{CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}")
#message("{CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}")
#
#get_filename_component(albert_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
#include("${albert_CMAKE_DIR}/AlbertTargets.cmake")

####################### nifty stuff for library developers

# outside because used by albert_plugin_generate_metadata_json and albert_plugin


function(albert_plugin_generate_metadata_json)
    set(MD "{}")
    string(JSON MD SET ${MD} "id" "\"${MD_ID}\"")
    string(JSON MD SET ${MD} "version" "\"${MD_VERSION}\"")
    string(JSON MD SET ${MD} "name" "\"${MD_NAME}\"")
    string(JSON MD SET ${MD} "description" "\"${MD_DESCRIPTION}\"")
    string(JSON MD SET ${MD} "license" "\"${MD_LICENSE}\"")
    string(JSON MD SET ${MD} "url" "\"${MD_URL}\"")

    if (MD_NOUSER)
        string(JSON MD SET ${MD} "type" "\"none\"")
    elseif(MD_FRONTEND)
        string(JSON MD SET ${MD} "type" "\"frontend\"")
    else()
        string(JSON MD SET ${MD} "type" "\"user\"")
    endif()

    if (DEFINED MD_MAINTAINERS)
        list(JOIN MD_MAINTAINERS "\", \"" X)
        string(JSON MD SET ${MD} "maintainers" "[\"${X}\"]")
    endif()
    if (DEFINED MD_QT_DEPENDENCIES)
        list(JOIN MD_QT_DEPENDENCIES "\", \"" X)
        string(JSON MD SET ${MD} "qt_deps" "[\"${X}\"]")
    endif()
    if (DEFINED MD_LIB_DEPENDENCIES)
        list(JOIN MD_LIB_DEPENDENCIES "\", \"" X)
        string(JSON MD SET ${MD} "lib_deps" "[\"${X}\"]")
    endif()
    if (DEFINED MD_EXEC_DEPENDENCIES)
        list(JOIN MD_EXEC_DEPENDENCIES "\", \"" X)
        string(JSON MD SET ${MD} "exec_deps" "[\"${X}\"]")
    endif()

    # Build authors list from git

    execute_process(COMMAND bash -c "git -C ${CMAKE_CURRENT_SOURCE_DIR} log --pretty=format:%an .|sort|uniq"
            OUTPUT_VARIABLE MD_AUTHORS OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "\n" ";" MD_AUTHORS ${MD_AUTHORS})
    if (DEFINED MD_AUTHORS)
        list(JOIN MD_AUTHORS "\", \"" X)
        string(JSON MD SET ${MD} "authors" "[\"${X}\"]")
    endif()

    # Create the metadata in the build dir
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" "${MD}")
    #    message("${CMAKE_CURRENT_BINARY_DIR}/metadata.json ${MD}")
endfunction()

function(albert_plugin_add_default_target)
    file(GLOB_RECURSE SRC *.cpp src/*.cpp *.qrc *.ui)
    #message("${PROJECT_NAME} ${SRC}")
    add_library(${PROJECT_NAME} SHARED ${SRC})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE albert::albert)
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
    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins
            )
endfunction()

function(albert_plugin_add_qt_dependencies)
    foreach(arg IN LISTS ARGN)
        find_package(Qt6 REQUIRED COMPONENTS ${arg})
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::${arg})
    endforeach()
endfunction()

function(albert_plugin)
    set(md_bool NOUSER FRONTEND)
    set(md_vals ID VERSION NAME DESCRIPTION LICENSE URL)
    set(md_list MAINTAINERS QT_DEPENDENCIES LIB_DEPENDENCIES EXEC_DEPENDENCIES)
    cmake_parse_arguments(MD "${md_bool}" "${md_vals}" "${md_list}" ${ARGV})

    if (NOT DEFINED MD_ID)
        get_filename_component(MD_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
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

    if (MD_NOUSER AND MD_FRONTEND)
        message(FATAL_ERROR "Conflict: NOUSER and FRONTEND specified.")
    endif()

    project(${MD_ID} VERSION ${MD_VERSION})

    albert_plugin_add_default_target()
    albert_plugin_add_default_target_properties()
    albert_plugin_generate_export_header()
    albert_plugin_generate_metadata_json()
    albert_plugin_add_qt_dependencies(${MD_QT_DEPENDENCIES})
    albert_plugin_add_default_install()
endfunction()
