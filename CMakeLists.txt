cmake_minimum_required(VERSION 3.16)  # Ubuntu 22.04

project(albert VERSION 0.17.6 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(GenerateExportHeader)

configure_file(
        "${PROJECT_SOURCE_DIR}/include/albert/config.h.in"
        "${PROJECT_SOURCE_DIR}/include/albert/config.h"
        @ONLY
)

find_package(Qt6 6.2  # Ubuntu 22.04
    REQUIRED
        Charts
        Core
        Concurrent
        Network
        Sql
        Widgets
)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winline  -Wextra -Wshadow -Wstrict-aliasing -pedantic -Wno-deprecated -Wno-unused-parameter") #-Wconversion -Weffc++ -Werror-Wall
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG")

# Color make output
SET(CMAKE_COLOR_MAKEFILE ON)

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#################################### Lib  #####################################

set(TARGET_LIB ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE LIB_SRC include/* src/lib/*.cpp src/lib/settingswidget/*.cpp)

add_library(${TARGET_LIB} SHARED ${LIB_SRC}
#        src/lib/app.cpp
#        src/lib/extension.cpp
#        src/lib/frontend.cpp
#        src/lib/history.cpp
#        src/lib/iconprovider.cpp
#        src/lib/indexqueryhandlerprivate.h
#        src/lib/itemindex.cpp
#        src/lib/levenshtein.cpp
#        src/lib/main.cpp
#        src/lib/plugin.cpp
#        src/lib/pluginprovider.cpp
#        src/lib/query.cpp
#        src/lib/queryengine.cpp
#        src/lib/queryhandler.cpp
#        src/lib/rpcserver.cpp
#        src/lib/scopedcrashindicator.hpp
#        src/lib/scopedtimeprinter.hpp
#        src/lib/trayicon.cpp
#        src/lib/usagehistory.cpp
#        src/lib/xdg/iconlookup.cpp
#        src/lib/xdg/themefileparser.cpp
)

add_library(albert::albert ALIAS ${TARGET_LIB})

generate_export_header(${TARGET_LIB}
    BASE_NAME ${CMAKE_PROJECT_NAME}
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/albert/export.h"
)

target_include_directories(${TARGET_LIB}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        include/albert
        include/albert/util
        src/lib
        src/lib/settingswidget
        src/lib/xdg
)

#if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    target_sources(${TARGET_LIB} PRIVATE src/lib/macos.mm)
#    target_link_libraries(${TARGET_LIB} PRIVATE objc "-framework Cocoa")
#endif()

target_link_libraries(${TARGET_LIB}
    PUBLIC
        Qt6::Core
    PRIVATE
        Qt6::Charts
        Qt6::Concurrent
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
)

set_target_properties(${TARGET_LIB} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

# Platform specific stuff

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        target_compile_definitions(${TARGET_LIB} PRIVATE -DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}")
    endif()

#    # Discriminate display server type
#    set(DISPLAY_SERVER "x11" CACHE STRING "Linux display server to build against (wayland or x11)." FORCE)
#    if(${DISPLAY_SERVER} MATCHES "x11")
#        find_package(Qt5 5.5.0 COMPONENTS X11Extras)
#        target_link_libraries(${TARGET_LIB} PRIVATE Qt5::X11Extras)
#    elseif(${DISPLAY_SERVER} MATCHES "wayland")
#    else()
#        message( FATAL_ERROR "Invalid display server specified: ${DISPLAY_SERVER}" )
#    endif()
endif()

#message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
#message("CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
#message("CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
#message("CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}")
#message("CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
#message("CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR}")
#message("CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}")

# associates the installed target files with an export called AlbertTargets
install(TARGETS ${TARGET_LIB}
    EXPORT AlbertTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# create version config file
write_basic_package_version_file("AlbertConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# copy config file, relocation aware
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AlbertConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/AlbertConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# install version and config file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AlbertConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AlbertConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/albert
)

# create targets file
install(EXPORT AlbertTargets
    FILE AlbertTargets.cmake
    NAMESPACE ${PROJECT_NAME}::
    #    DESTINATION "cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Exporting Targets from the Build Tree
export(EXPORT AlbertTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/AlbertTargets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)

# TODO this does works for the build tree, installation has never been tested
## install headers
#file(GLOB_RECURSE TARGET_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
#install(FILES ${TARGET_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#################################### App  #####################################

set(TARGET_APP ${CMAKE_PROJECT_NAME}app)

add_executable(${TARGET_APP} src/app/main.cpp )

target_link_libraries(${TARGET_APP}
    PRIVATE ${TARGET_LIB}
)

set_target_properties(${TARGET_APP} PROPERTIES
    OUTPUT_NAME ${CMAKE_PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
)

# Platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    # Install desktop entry and icon
    install(FILES dist/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
    install(FILES resources/icons/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

install(TARGETS ${TARGET_APP} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#################################### Ctl  #####################################

set(TARGET_CTL ${CMAKE_PROJECT_NAME}ctl)
add_executable(${TARGET_CTL} src/ctl/main.cpp)
target_link_libraries(${TARGET_CTL} PRIVATE Qt6::Network)
install(TARGETS ${TARGET_CTL} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#################################### Test #####################################

#option(BUILD_TESTS "Build tests (Reqiures doctest)" ON)
#if (BUILD_TESTS)
#    set(TARGET_TST ${CMAKE_PROJECT_NAME}_test)
#    find_package(doctest)
#    add_executable(${TARGET_TST}
#            src/lib/levenshtein.h
#            src/lib/levenshtein.cpp
#            src/lib/staticitemindex.h
#            src/lib/staticitemindex.cpp
#            test/test.cpp
#    )
#    target_include_directories(${TARGET_TST} PRIVATE include src)
#    target_link_libraries(${TARGET_TST} PRIVATE albert::lib doctest::doctest Qt5::Core)
#endif()

################################# Plugins  ####################################
#
#FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/plugins/ plugins/*)
#LIST(FILTER children EXCLUDE REGEX "^\\..+")
#FOREACH(child ${children})
#    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugins/${child})
#        string(TOUPPER ${child} CHILD)
#        option(BUILD_${CHILD} "Build ${child}" ON)
#        if (BUILD_${CHILD})
#            add_subdirectory(plugins/${child})
#        endif()
#    ENDIF()
#ENDFOREACH()