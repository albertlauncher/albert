#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.12)

project(appLauncher)

SET(TARGET_VERSION_MAJOR 1)
SET(TARGET_VERSION_MINOR 0)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra")

find_package(Qt5Widgets 5.1 REQUIRED)
MESSAGE(STATUS "Qt5Widgets_VERSION " ${Qt5Widgets_VERSION_STRING})

find_package(Qt5Svg REQUIRED)
MESSAGE(STATUS "Qt5Svg_VERSION " ${Qt5Svg_VERSION})

include_directories(
	../../src/common
	src/
)

set(APPLAUNCHER_SRCS
    src/metadata.json
    src/applauncher.cpp
	src/configwidget.cpp
	../../src/common/extensioninterface.h
	../../src/common/abstractsearch.h
	../../src/common/prefixsearch.h
#	../../src/common/fuzzysearch.cpp
)

qt5_wrap_ui(APPLAUNCHER_UIS
	src/configwidget.ui
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	ADD_LIBRARY(applauncher SHARED ${APPLAUNCHER_SRCS} ${APPLAUNCHER_UIS})
	SET_TARGET_PROPERTIES(applauncher PROPERTIES LINKER_LANGUAGE C)
	target_link_libraries(applauncher Qt5::Widgets)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
