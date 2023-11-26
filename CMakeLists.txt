cmake_minimum_required(VERSION 3.19)  # JSON support, Ubuntu 22.04

# dont touch! set by metatool
set(PROJECT_VERSION 0.22.17)

project(albert
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Keyboard launcher"
    HOMEPAGE_URL "https://albertlauncher.github.io"
)
set(PROJECT_DISPLAY_NAME "Albert")

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

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
    -Werror=return-type \
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

### QHotkey  ##################################################################

set(QT_DEFAULT_MAJOR_VERSION 6)
set(BUILD_SHARED_LIBS FALSE)
set(QHOTKEY_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/QHotkey EXCLUDE_FROM_ALL)
target_compile_options(qhotkey PRIVATE -Wno-unused-parameter -Wno-shadow)


### Lib  ######################################################################

set(TARGET_LIB lib${PROJECT_NAME})

file(GLOB_RECURSE LIB_PUBLIC_HEADER
    ${PROJECT_BINARY_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.h
)

file(GLOB_RECURSE LIB_SRC_UI src/*.ui)

file(GLOB LIB_SRC
    src/*.h
    src/*.cpp
    src/platform/*.h
    src/settings/*.h
    src/settings/*.cpp
)

file(GLOB_RECURSE LIB_SRC_PLATFORM
    src/platform/${CMAKE_SYSTEM_NAME}/*.h
    src/platform/${CMAKE_SYSTEM_NAME}/*.cpp
    src/platform/${CMAKE_SYSTEM_NAME}/*.mm
)

add_library(${TARGET_LIB} SHARED
    ${LIB_PUBLIC_HEADER}
    ${LIB_SRC_PLATFORM}
    ${LIB_SRC_UI}
    ${LIB_SRC}
    src/resources.qrc
)

add_library(albert::albert ALIAS ${TARGET_LIB})

include(GenerateExportHeader)
generate_export_header(${TARGET_LIB}
    BASE_NAME ${PROJECT_NAME}
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/albert/export.h"
)

target_include_directories(${TARGET_LIB}
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        src/
)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/albert/config.h.in"
    "${PROJECT_BINARY_DIR}/include/albert/config.h"
    @ONLY
)

find_package(Qt6 6.2 REQUIRED  # Ubuntu 22.04
    Core
    Concurrent
    Network
    Sql
    Svg
    Widgets
)

target_link_libraries(${TARGET_LIB}
    PUBLIC
        Qt6::Core
    PRIVATE
        Qt6::Widgets
        Qt6::Concurrent
        Qt6::Network
        Qt6::Sql
        qhotkey
)

set_target_properties(${TARGET_LIB} PROPERTIES
    #INSTALL_RPATH "$ORIGIN"
    CXX_VISIBILITY_PRESET hidden
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_FRAMEWORK_IDENTIFIER "org.albertlauncher.albert.interface"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
    OUTPUT_NAME "${PROJECT_NAME}"
    PUBLIC_HEADER "${LIB_PUBLIC_HEADER}"
    SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"  # also mach-o compat version
    # NO_SONAME true # do _not_ add. linkers other that ldd will make troubles using non relative paths
    VERSION ${PROJECT_VERSION}
    VISIBILITY_INLINES_HIDDEN 1
)

### Platform specifics ########################################################

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        target_compile_definitions(${TARGET_LIB} PRIVATE -DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}")
        target_link_libraries(${TARGET_LIB} PRIVATE Qt6::DBus)  # notifications
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${TARGET_LIB}
        PRIVATE
        objc
        "-framework Cocoa"
        #"-framework UserNotifications"
    )
endif()

if (UNIX)
    target_sources(${TARGET_LIB} PRIVATE
        src/platform/Unix/unixsignalhandler.h
        src/platform/Unix/unixsignalhandler.cpp
    )
endif (UNIX)


### App  ######################################################################

set(TARGET_APP ${CMAKE_PROJECT_NAME})

add_executable(${TARGET_APP} MACOSX_BUNDLE "src/main.cpp")

# Platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    install(FILES dist/xdg/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
    install(FILES resources/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    set_target_properties(${TARGET_APP} PROPERTIES MACOSX_BUNDLE TRUE)

    set(ICON_NAME "albert")
    set(ICON_PATH "dist/macos/${ICON_NAME}.icns")
    target_sources(${TARGET_APP} PRIVATE ${ICON_PATH})

    set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(${TARGET_APP} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_DISPLAY_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2023 Manuel Schneider"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.albertlauncher.albert"
        MACOSX_BUNDLE_ICON_FILE ${ICON_NAME}
        MACOSX_BUNDLE_INFO_STRING "Albert keyboard launcher"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    )

endif()

target_link_libraries(${TARGET_APP}
    PRIVATE ${TARGET_LIB}
)

set_target_properties(${TARGET_APP} PROPERTIES
    OUTPUT_NAME ${CMAKE_PROJECT_NAME}
    #INSTALL_RPATH_USE_LINK_PATH TRUE
    #INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    #BUILD_WITH_INSTALL_RPATH FALSE
    #INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
    #INSTALL_RPATH "$ORIGIN/../Frameworks/albert/"  # Set the RPATH for the library lookup
)


### Test ######################################################################

option(BUILD_TESTS "Build tests (Reqiures doctest)" OFF)
if (BUILD_TESTS)
    set(TARGET_TST ${CMAKE_PROJECT_NAME}_test)
    find_package(doctest)

#    get_target_property(SRC_TST ${TARGET_LIB} SOURCES)
#    get_target_property(INC_TST ${TARGET_LIB} PRIVATE_HEADER)

    add_executable(${TARGET_TST}
        test/test.cpp
        src/itemindex.cpp
        src/levenshtein.cpp
        test/test.cpp
    )
    target_link_libraries(${TARGET_TST} PRIVATE ${TARGET_LIB})
    target_include_directories(${TARGET_TST} PRIVATE include src)
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


### Install ###################################################################

install(
    TARGETS ${TARGET_APP} ${TARGET_LIB}
    TARGETS EXPORT ${EXPORT_TARGETS}
    BUNDLE    DESTINATION .
    FRAMEWORK DESTINATION .
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}
)



#message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
#message("CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
#message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
#message("PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}")
#message("CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
#message("CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}")
#message("CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
#message("CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR}")
#message("CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}")

#set (EXPORT_TARGETS AlbertTargets)

## install headers
#file(GLOB_RECURSE
#    TARGET_HEADER_FILES
#    "${PROJECT_SOURCE_DIR}/include/*.h"
#    "${PROJECT_BINARY_DIR}/include/*.h"
#)
#message(${TARGET_HEADER_FILES})
#install(DIRECTORY mydir/ DESTINATION dir/newname)
#install(FILES ${TARGET_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

## Create targets file in install tree
#install(
#    EXPORT ${EXPORT_TARGETS}
#    FILE "${PROJECT_NAME}-targets.cmake"
#    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}-${PROJECT_VERSION}
#    NAMESPACE ${PROJECT_NAME}::
#)

## Create targets file in build tree
#export(
#    EXPORT ${EXPORT_TARGETS}
#    FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake"
#    NAMESPACE ${PROJECT_NAME}::
#)

## Create config file in build tree
#include(CMakePackageConfigHelpers)
#configure_package_config_file(
#    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
#    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
#)

## Create version config file in build tree
#write_basic_package_version_file(
#    "${PROJECT_NAME}-config-version.cmake"
#    VERSION ${PROJECT_VERSION}
#    COMPATIBILITY SameMajorVersion
#)

# install version and config file
#install(FILES
#    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
#    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-macros.cmake"
#    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/albert
#)

## Copy macros to build tree
#configure_file(
#    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-macros.cmake"
#    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-macros.cmake"
#    COPYONLY
#)

# Export the package to make find_package work in build
#set(CMAKE_EXPORT_PACKAGE_REGISTRY true)
#export(PACKAGE ${PROJECT_NAME})




### Packaging #################################################################

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_CHECKSUM "SHA256")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_EXECUTABLES "albert" "Albert executable")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_DISPLAY_NAME}-v${PROJECT_VERSION}")
    set(CPACK_PACKAGE_NAME "${PROJECT_DISPLAY_NAME}")
    #set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/${ICON_PATH}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "manuelschneid3r")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.md")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_MONOLITHIC_INSTALL TRUE)

    configure_file("cmake/bundle-macos.cmake.in" "${PROJECT_BINARY_DIR}/bundle-macos.cmake" @ONLY)
    install(SCRIPT "${PROJECT_BINARY_DIR}/bundle-macos.cmake")

    include(CPack)
endif()
