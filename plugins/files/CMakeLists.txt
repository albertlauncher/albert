#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.11)

project(files)

SET(TARGET_VERSION_MAJOR 1)
SET(TARGET_VERSION_MINOR 0)

include(../../global_cmake_settings.txt REQUIRED)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
	src/
        src/actions
        ../../src/interfaces/
)

set(${PROJECT_NAME}_SRCS
    src/metadata.json

    src/extension.cpp
    src/item.cpp
    src/configwidget.cpp

    src/file.h
    src/scanworker.cpp
    src/mimetypedialog.cpp
    src/mimetypechooser.cpp
    src/actions/openfileaction.cpp
    src/actions/revealfileaction.cpp
    src/actions/copyfileaction.cpp
    src/actions/copypathaction.cpp
)

qt5_wrap_ui(${PROJECT_NAME}_UIS
    src/configwidget.ui
    src/mimetypedialog.ui
    src/mimetypechooser.ui
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_UIS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${PROJECT_NAME} ${Qt5Widgets_LIBRARIES} )
