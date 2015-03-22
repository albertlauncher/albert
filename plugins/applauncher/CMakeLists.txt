cmake_minimum_required(VERSION 2.8.12)
project(appLauncher)
SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE} )

find_package(Qt5Widgets REQUIRED)

include_directories(
	../../src/common
	src/
)

set(APPLAUNCHER_SRCS
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
