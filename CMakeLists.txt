cmake_minimum_required(VERSION 3.19)  # JSON support, Ubuntu 22.04

project(albert VERSION 0.18.0 LANGUAGES CXX HOMEPAGE_URL "albertlauncher.github.io")

include(GNUInstallDirs)

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Winline \
    -Wextra \
    -Wshadow \
    -Wstrict-aliasing \
    -pedantic \
    -Wno-deprecated \
    -Wno-unused-parameter \
    -Wno-shadow \
")
#    -Wconversion \
#    -Weffc++ \
#    -Werror \
#    -Wall \

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

### Lib  ######################################################################

set(TARGET_LIB ${PROJECT_NAME})

file(GLOB_RECURSE LIB_SRC include/*)

set (LIB_SRC ${LIB_SRC}
        include/albert/util/timeprinter.hpp
        src/lib/albert.cpp
        src/lib/app.cpp
        src/lib/extension.cpp
        src/lib/extensionregistry.cpp
        src/lib/extensions/frontend.cpp
        src/lib/extensions/globalqueryhandler.cpp
        src/lib/extensions/indexqueryhandler.cpp
        src/lib/extensions/pluginprovider.cpp
        src/lib/extensions/queryhandler.cpp
        src/lib/globalsearch.cpp
        src/lib/iconprovider.cpp
        src/lib/itemindex.cpp
        src/lib/itemsmodel.cpp
        src/lib/levenshtein.cpp
        src/lib/nativepluginprovider.cpp
        src/lib/pluginregistry.cpp
        src/lib/query.cpp
        src/lib/queryengine.cpp
        src/lib/resources.qrc
        src/lib/rpcserver.cpp
        src/lib/scopedcrashindicator.hpp
        src/lib/settings/configproviderwidget.cpp
        src/lib/settings/pluginwidget.cpp
        src/lib/settings/settingswindow.cpp
        src/lib/settings/settingswindow.ui
        src/lib/settings/triggerwidget.cpp
        src/lib/terminalprovider.cpp
        src/lib/trayicon.cpp
        src/lib/usagedatabase.cpp
        src/lib/util/history.cpp
        src/lib/util/standarditem.cpp
        src/lib/util/util.cpp
        src/lib/xdg/iconlookup.cpp
        src/lib/xdg/themefileparser.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (LIB_SRC ${LIB_SRC} src/lib/macos.mm )
endif()

add_library(${TARGET_LIB} SHARED ${LIB_SRC})

add_library(albert::albert ALIAS ${TARGET_LIB})

include(GenerateExportHeader)
generate_export_header(${TARGET_LIB}
    BASE_NAME ${PROJECT_NAME}
    EXPORT_FILE_NAME "${PROJECT_SOURCE_DIR}/include/albert/export.h"
)

target_include_directories(${TARGET_LIB}
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        src/lib
        src/lib/settingswidget
        src/lib/xdg
)

configure_file(
        "${PROJECT_SOURCE_DIR}/include/albert/config.h.in"
        "${PROJECT_SOURCE_DIR}/include/albert/config.h"
        @ONLY
)

find_package(Qt6 6.2 REQUIRED  # Ubuntu 22.04
    Core
    Concurrent
    Network
    Sql
    Widgets
)

target_link_libraries(${TARGET_LIB}
    PUBLIC
        Qt6::Core
        Qt6::Widgets
    PRIVATE
        Qt6::Concurrent
        Qt6::Network
        Qt6::Sql
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

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    target_sources(${TARGET_LIB} PRIVATE src/lib/macos.mm)
    target_link_libraries(${TARGET_LIB} PRIVATE objc "-framework Cocoa")

endif()

#message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
#message("CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
#message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
#message("PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}")
#message("CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
#message("CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}")
#message("CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
#message("CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR}")
#message("CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}")

install(
    TARGETS ${TARGET_LIB}
    EXPORT TARGET_LIB_EXPORTS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Create targets file in install tree (Todo untested)
install(
    EXPORT TARGET_LIB_EXPORTS
    FILE "${PROJECT_NAME}-targets.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}-${PROJECT_VERSION}
    NAMESPACE ${PROJECT_NAME}::
)

# Create targets file in build tree
export(
    EXPORT TARGET_LIB_EXPORTS
    FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)

# Create config file in build tree
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
)

# Create version config file in build tree
write_basic_package_version_file(
    "${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# install version and config file
install(FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-macros.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/albert
)

# Copy macros to build tree
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-macros.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-macros.cmake"
    COPYONLY
)

# Export the package to make find_package work in build
set(CMAKE_EXPORT_PACKAGE_REGISTRY true)
export(PACKAGE ${PROJECT_NAME})

# TODO this does works for the build tree, installation has never been tested
## install headers
#file(GLOB_RECURSE TARGET_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/*.h")
#install(FILES ${TARGET_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

### App  ######################################################################

set(TARGET_APP ${CMAKE_PROJECT_NAME}app)

# Platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    add_executable(${TARGET_APP} "src/app/main.cpp")

    install(FILES dist/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
    install(FILES resources/icons/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    set(ICON_NAME "AppIcon")
    set(ICON_PATH "resources/${ICON_NAME}.icns")
    message(${ICON_PATH})

    add_executable(${TARGET_APP} MACOSX_BUNDLE "${ICON_PATH}" "src/app/main.cpp")

    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(${TARGET_APP} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_BUNDLE_NAME "Albert"
        MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2022 Manuel Schneider"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.albertlauncher.albert"
        MACOSX_BUNDLE_ICON_FILE ${ICON_NAME}
        MACOSX_BUNDLE_INFO_STRING "Albert keyboard launcher"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in
    )

endif()

target_link_libraries(${TARGET_APP}
    PRIVATE ${TARGET_LIB}
)

set_target_properties(${TARGET_APP} PROPERTIES
    OUTPUT_NAME ${CMAKE_PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
)

install(TARGETS ${TARGET_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)



### Ctl #######################################################################

set(TARGET_CTL ${CMAKE_PROJECT_NAME}ctl)
add_executable(${TARGET_CTL} src/ctl/main.cpp)
target_link_libraries(${TARGET_CTL} PRIVATE Qt6::Network)
install(TARGETS ${TARGET_CTL} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

### Test ######################################################################

option(BUILD_TESTS "Build tests (Reqiures doctest)" OFF)
if (BUILD_TESTS)
    set(TARGET_TST ${CMAKE_PROJECT_NAME}_test)
    find_package(doctest)
    add_executable(${TARGET_TST}
            src/lib/extensions/globalqueryhandler.cpp
            src/lib/extensions/indexqueryhandler.cpp
            src/lib/extensions/queryhandler.cpp
            src/lib/itemindex.cpp
            src/lib/levenshtein.cpp
            src/lib/util/standarditem.cpp
            test/test.cpp
    )
    target_include_directories(${TARGET_TST} PRIVATE include src/lib)
    target_link_libraries(${TARGET_TST} PRIVATE
            doctest::doctest
            Qt6::Core
            Qt6::Concurrent
            Qt6::Sql
            )
endif()

### Plugins ###################################################################

include("cmake/albert-macros.cmake")

FILE(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/plugins/ plugins/*)
LIST(FILTER children EXCLUDE REGEX "^\\..+")
FOREACH(child ${children})
    IF(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/plugins/${child})
        string(TOUPPER ${child} CHILD)
        option(BUILD_${CHILD} "Build ${child}" ON)
        if (BUILD_${CHILD})
            add_subdirectory(plugins/${child})
        endif()
    ENDIF()
ENDFOREACH()


### Packaging #################################################################

add_subdirectory(dist)
