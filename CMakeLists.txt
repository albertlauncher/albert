cmake_minimum_required(VERSION 3.1.3)  # Least supporting CMAKE_CXX_STANDARD

include(GenerateExportHeader)

###############################################################################

project(albert VERSION 0.17.6)

configure_file(
        "${PROJECT_SOURCE_DIR}/include/albert/config.h.in"
        "${PROJECT_SOURCE_DIR}/include/albert/config.h"
        @ONLY
)

find_package(Qt5 5.6.0  # Charts
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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winline -Wall -Wextra -Wshadow -Wstrict-aliasing -pedantic -Wno-deprecated -Wno-unused-parameter") #-Wconversion -Weffc++ -Werror
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG")

# Color make output
SET(CMAKE_COLOR_MAKEFILE ON)

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(lib/xdg)

#################################### Lib  #####################################

set(TARGET_LIB lib${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE LIB_SRC include/* src/lib/*.cpp src/lib/settingswidget/*.cpp)

add_library(${TARGET_LIB} SHARED ${LIB_SRC})

add_library(albert::lib ALIAS ${TARGET_LIB})

generate_export_header(${TARGET_LIB}
    BASE_NAME ${CMAKE_PROJECT_NAME}
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/albert/export.h"
)

target_include_directories(${TARGET_LIB}
    PUBLIC
        include
    PRIVATE
        include/albert
        include/albert/util
        src/lib
        src/lib/settingswidget
)

#if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    target_sources(${TARGET_LIB} PRIVATE src/lib/macos.mm)
#    target_link_libraries(${TARGET_LIB} PRIVATE objc "-framework Cocoa")
#endif()

target_link_libraries(${TARGET_LIB}
    PUBLIC
        Qt5::Core
    PRIVATE
        Qt5::Charts
        Qt5::Concurrent
        Qt5::Network
        Qt5::Sql
        Qt5::Widgets
        albert::xdg
)

set_target_properties(${TARGET_LIB} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

# Platform specific stuff

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    include(GNUInstallDirs)
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

#################################### App  #####################################

set(TARGET_APP ${CMAKE_PROJECT_NAME})

add_executable(${TARGET_APP} src/app/main.cpp )

target_link_libraries(${TARGET_APP}
    PRIVATE albert::lib
)

set_target_properties(${TARGET_APP} PROPERTIES
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
)

# Platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    # Install desktop entry and icon
    install(FILES resources/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
    install(FILES resources/icons/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

#################################### Ctl  #####################################
#
#set(TARGET_CTL ${CMAKE_PROJECT_NAME}ctl)
#add_executable(${TARGET_CTL} src/ctl/main.cpp)
#target_link_libraries(${TARGET_CTL} PRIVATE Qt5::Network)

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

FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/plugins/ plugins/*)
LIST(FILTER children EXCLUDE REGEX "^\\..+")
FOREACH(child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugins/${child})
        string(TOUPPER ${child} CHILD)
        option(BUILD_${CHILD} "Build ${child}" ON)
        if (BUILD_${CHILD})
            add_subdirectory(plugins/${child})
        endif()
    ENDIF()
ENDFOREACH()

###############################################################################

install(TARGETS ${TARGET_APP} ${TARGET_CTL} ${TARGET_LIB}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION include
)


