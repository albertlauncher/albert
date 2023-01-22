cmake_minimum_required(VERSION 3.19)  # string(JSON…

macro(albert_plugin_generate_metadata_json)
    set(MD "{}")
    string(JSON MD SET ${MD} "id" "\"${PROJECT_NAME}\"")
    string(JSON MD SET ${MD} "version" "\"${PROJECT_VERSION}\"")
    string(JSON MD SET ${MD} "name" "\"${MD_NAME}\"")
    string(JSON MD SET ${MD} "description" "\"${MD_DESCRIPTION}\"")

    if(EXISTS ${MD_LONG_DESCRIPTION})
        file(READ ${MD_LONG_DESCRIPTION} MD_LONG_DESCRIPTION)
    endif()
    string(JSON MD SET ${MD} "long_description" "\"${MD_LONG_DESCRIPTION}\"")
    string(JSON MD SET ${MD} "license" "\"${MD_LICENSE}\"")
    string(JSON MD SET ${MD} "url" "\"${MD_URL}\"")

    if(MD_FRONTEND)
        string(JSON MD SET ${MD} "frontend" "true")
    else()
        string(JSON MD SET ${MD} "frontend" "false")
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

    if (DEFINED MD_CREDITS)
        list(JOIN MD_CREDITS "\", \"" X)
        string(JSON MD SET ${MD} "credits" "[\"${X}\"]")
    endif()

    # Create the metadata in the build dir
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" "${MD}")
    #message("${CMAKE_CURRENT_BINARY_DIR}/metadata.json ${MD}")
endmacro()

macro(albert_plugin_add_default_target)
    file(GLOB_RECURSE SRC src/*.h src/*.cpp src/*.mm *.qrc *.ui )
    #message("${PROJECT_NAME} ${SRC}")
    add_library(${PROJECT_NAME} SHARED ${SRC})
    add_library(albert::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE albert::albert)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DPROJECT_NAME="${PROJECT_NAME}")  # logging.h
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
        INSTALL_RPATH "$ORIGIN"
    )
    include(GenerateExportHeader)
    generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME "export.h")
    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    )
    foreach(arg IN LISTS MD_QT_DEPENDENCIES)
        find_package(Qt6 REQUIRED COMPONENTS ${arg})
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::${arg})
    endforeach()
endmacro()

macro(albert_plugin)
    set(md_bool FRONTEND)
    set(md_vals NAME DESCRIPTION LONG_DESCRIPTION LICENSE URL)
    set(md_list MAINTAINERS QT_DEPENDENCIES LIB_DEPENDENCIES EXEC_DEPENDENCIES CREDITS)
    cmake_parse_arguments(MD "${md_bool}" "${md_vals}" "${md_list}" ${ARGV})

    if (NOT DEFINED PROJECT_VERSION)
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

    albert_plugin_add_default_target()
    albert_plugin_generate_metadata_json()
endmacro()

