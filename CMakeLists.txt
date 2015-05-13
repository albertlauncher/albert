#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.12)

PROJECT(albert)

SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
enable_testing()

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -Wconversion -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT ")

find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
    src/
    src/common/
    src/hotkey/
    src/mainwidget/
    src/settingswidget/
)

set(SRCS
    TODO
    src/main.cpp
    src/extensionhandler.cpp
    src/pluginhandler.cpp
    src/hotkey/hotkeymanager.cpp
    src/hotkey/hotkeymanager_p.h
    src/mainwidget/mainwidget.cpp
    src/mainwidget/proposallist.cpp
    src/mainwidget/inputline.cpp
    src/settingswidget/grabkeybutton.cpp
    src/settingswidget/settingsbutton.h
    src/settingswidget/settingswidget.cpp
    src/common/objects.h
    src/common/query.cpp
    src/common/plugininterfaces/plugin_if.h
    src/common/plugininterfaces/extension_if.h
    src/common/plugininterfaces/extensionprovider_if.h
    src/common/search/search.h
    src/common/search/search_impl.h
    src/common/search/fuzzysearch.h
    src/common/search/prefixsearch.h
)

set(LIBS
    ${Qt5Widgets_LIBRARIES}
)

#set(data
#	data/icons/youtube.svg
#	data/icons/google.svg
#	data/icons/ebay.svg
#	data/icons/amazon.svg
#)

qt5_wrap_ui(UIS
    src/settingswidget/settingswidget.ui
    src/mainwidget/mainwidget.ui
#	src/util/search/searchwidget.ui
#	src/services/appindex/appindexwidget.ui
#	src/services/bookmarkindex/bookmarkindexwidget.ui
#	src/services/calculator/calculatorwidget.ui
#	src/services/fileindex/fileindexwidget.ui
#	src/services/websearch/websearchwidget.ui
)

qt5_add_resources(RSRC
    src/resources.qrc
)


# Do platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Qt5X11Extras 5.1 REQUIRED)
    set(SRCS ${SRCS} src/hotkey/hotkeymanager_x11.cpp)
    set(LIBS ${LIBS} ${Qt5X11Extras_LIBRARIES} X11)
    install(FILES src/resources/albert.desktop DESTINATION share/applications COMPONENT desktop)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SRCS ${SRCS} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")


## Define targets and linked libraries
add_executable(${PROJECT_NAME} ${SRCS} ${UIS} ${DATA} ${RSRC})
target_link_libraries(${PROJECT_NAME} ${LIBS})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY src/resources/themes DESTINATION share/${PROJECT_NAME} COMPONENT config)
# TODO PLUGINS?


# Build plugins
add_subdirectory(plugins/files)
#add_subdirectory(plugins/applications)
#add_subdirectory(plugins/chromebookmarks)
#add_subdirectory(plugins/websearch)
#add_subdirectory(plugins/PythonExtensions)
