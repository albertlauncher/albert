#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.12)

project(files)

SET(TARGET_VERSION_MAJOR 1)
SET(TARGET_VERSION_MINOR 0)

# Find includes in corresponding build directories
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wconversion -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT ")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
	src/
	../../src/common
)

set(${PROJECT_NAME}_SRCS
    src/metadata.json

    src/extension.cpp
	src/configwidget.cpp
	src/file.cpp
	src/actions.cpp
	src/scanworker.cpp
	src/mimetypedialog.cpp
	src/mimetypechooser.cpp

	../../src/common/objects.h
	../../src/common/search/search.h
)

qt5_wrap_ui(${PROJECT_NAME}_UIS
	src/configwidget.ui
	src/mimetypedialog.ui
	src/mimetypechooser.ui
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_UIS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${PROJECT_NAME} ${Qt5Widgets_LIBRARIES} )
