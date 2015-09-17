#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.11)

PROJECT(albert)
SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)


include(./global_cmake_settings.txt OPTIONAL)

find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
    src/
    src/hotkey/
    src/mainwidget/
    src/settingswidget/
)

set(SRCS
    TODO
    src/main.cpp
    src/albertapp.cpp
    src/extensionhandler.cpp
    src/pluginloader.cpp
    src/pluginhandler.cpp
    src/query.cpp
    src/interfaces.h
    src/utils/search/search.h
    src/utils/search/search_impl.h
    src/utils/search/fuzzysearch.h
    src/utils/search/prefixsearch.h
    src/utils/search/prefixsearch.h
    src/hotkey/hotkeymanager.cpp
    src/hotkey/hotkeymanager_p.h
    src/mainwidget/mainwidget.cpp
    src/mainwidget/proposallist.cpp
    src/mainwidget/itemdelegate.cpp
    src/mainwidget/inputline.cpp
    src/settingswidget/grabkeybutton.cpp
    src/settingswidget/settingsbutton.h
    src/settingswidget/settingswidget.cpp
)

set(LIBS
    ${Qt5Widgets_LIBRARIES}
)

qt5_wrap_ui(UIS
    src/settingswidget/settingswidget.ui
    src/mainwidget/mainwidget.ui
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
