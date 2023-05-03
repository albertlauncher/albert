cmake_minimum_required(VERSION 3.19)  # JSON support, Ubuntu 22.04

project(albert VERSION 0.21.0)

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
    -Wall \
    -Wno-inline \
")
#    -Werror \
#    -Wshadow-field-in-constructor \
#    -Wno-unused-parameter \
#    -Wno-shadow \
#    -Wl,-undefined \
#    -Weffc++
#    -Wconversion \
#    -Wno-deprecated \
#string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,-undefined,error")  #macosonly

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

### Lib  ######################################################################

set(QT_DEFAULT_MAJOR_VERSION 6)
set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(lib/QHotkey EXCLUDE_FROM_ALL)
target_compile_options(qhotkey PRIVATE -Wno-unused-parameter -Wno-shadow)

set(TARGET_LIB ${PROJECT_NAME})

file(GLOB_RECURSE LIB_PUBLIC_HEADER include/*.h)

set (LIB_UI
    src/settings/settingswindow.ui
#    src/settings/pluginwidget.ui
)

set (LIB_SRC
    include/albert/util/timeprinter.hpp
    src/albert.cpp
    src/app.cpp
    src/app.h
    src/extension.cpp
    src/extensionregistry.cpp
    src/extensions/frontend.cpp
    src/extensions/globalqueryhandlerprivate.cpp
    src/extensions/globalqueryhandlerprivate.h
    src/extensions/indexqueryhandlerprivate.cpp
    src/extensions/indexqueryhandlerprivate.h
    src/extensions/item.cpp
    src/extensions/pluginprovider.cpp
    src/extensions/queryhandler.cpp
    src/globalsearch.cpp
    src/globalsearch.h
    src/hotkey.cpp
    src/hotkey.h
    src/iconprovider.cpp
    src/iconprovider.h
    src/index.h
    src/itemindex.cpp
    src/itemindex.h
    src/itemsmodel.cpp
    src/itemsmodel.h
    src/levenshtein.cpp
    src/levenshtein.h
    src/nativepluginprovider.cpp
    src/nativepluginprovider.h
    src/pluginregistry.cpp
    src/pluginregistry.h
    src/query.cpp
    src/query.h
    src/queryengine.cpp
    src/queryengine.h
    src/rpcserver.cpp
    src/rpcserver.h
    src/scopedcrashindicator.cpp
    src/scopedcrashindicator.h
    src/settings/pluginwidget.cpp
    src/settings/pluginwidget.h
    src/settings/settingswindow.cpp
    src/settings/settingswindow.h
    src/settings/triggerwidget.cpp
    src/settings/triggerwidget.h
    src/telemetry.cpp
    src/telemetry.h
    src/terminalprovider.cpp
    src/terminalprovider.h
    src/trayicon.cpp
    src/trayicon.h
    src/usagedatabase.cpp
    src/usagedatabase.h
    src/util/history.cpp
    src/util/standarditem.cpp
    src/util/util.cpp
    src/xdg/iconlookup.cpp
    src/xdg/iconlookup.h
    src/xdg/themefileparser.cpp
    src/xdg/themefileparser.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (LIB_SRC ${LIB_SRC} src/macos.mm src/macos.h )
endif()

add_library(${TARGET_LIB} SHARED ${LIB_SRC} ${LIB_UI} ${LIB_PUBLIC_HEADER} src/resources.qrc)

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
        src/
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
        qhotkey
)

set_target_properties(${TARGET_LIB} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
#    PUBLIC_HEADER "${LIB_PUBLIC_HEADER}"
)

# Platform specific stuff

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        target_compile_definitions(${TARGET_LIB} PRIVATE -DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}")
    endif()

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

#    target_sources(${TARGET_LIB} PRIVATE src/macos.mm)
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

add_executable(${TARGET_APP} "src/main.cpp")

# Platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    install(FILES dist/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
    install(FILES resources/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    set_target_properties(${TARGET_APP} PROPERTIES MACOSX_BUNDLE TRUE)

    set(ICON_NAME "albert")
    set(ICON_PATH "resources/${ICON_NAME}.icns")

#    add_executable(${TARGET_APP} MACOSX_BUNDLE "${ICON_PATH}" "src/app/main.cpp")

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
   # INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
)

install(TARGETS ${TARGET_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)

### Test ######################################################################

option(BUILD_TESTS "Build tests (Reqiures doctest)" OFF)
if (BUILD_TESTS)
    set(TARGET_TST ${CMAKE_PROJECT_NAME}_test)
    find_package(doctest)
    add_executable(${TARGET_TST}
            src/extensions/globalqueryhandler.cpp
            src/extensions/indexqueryhandler.cpp
            src/extensions/queryhandler.cpp
            src/itemindex.cpp
            src/levenshtein.cpp
            src/util/standarditem.cpp
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
