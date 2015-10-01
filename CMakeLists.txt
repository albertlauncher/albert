#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.11)

PROJECT(albert)
SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

include(./global_cmake_settings.txt REQUIRED)

find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
    src/
    src/hotkey/
    src/mainwidget/
    src/interfaces/
    src/pluginsystem/
    src/pluginsystem/extension/
    src/settingswidget/
)

set(SRCS
    TODO

    src/main.cpp
    src/albertapp.cpp

    src/hotkey/hotkeymanager.cpp
    src/hotkey/hotkeymanager_p.h

    src/mainwidget/inputline.cpp
    src/mainwidget/itemdelegate.cpp
    src/mainwidget/mainwidget.cpp
    src/mainwidget/proposallist.cpp
    src/mainwidget/settingsbutton.h

    src/pluginsystem/pluginloader.cpp
    src/pluginsystem/pluginmanager.cpp

    src/pluginsystem/extension/extensionmanager.cpp
    src/pluginsystem/extension/query.cpp
    src/pluginsystem/extension/query_p.h

    src/pluginsystem/extension/interfaces/iextension.h
    src/pluginsystem/extension/interfaces/baseobjects.h

    src/pluginsystem/extension/utils/search/fuzzysearch.h
    src/pluginsystem/extension/utils/search/iindexable.h
    src/pluginsystem/extension/utils/search/prefixsearch.h
    src/pluginsystem/extension/utils/search/search.h
    src/pluginsystem/extension/utils/search/search_impl.h

    src/settingswidget/grabkeybutton.cpp
    src/settingswidget/settingswidget.cpp
)

set(LIBS
    ${Qt5Widgets_LIBRARIES}
)

qt5_wrap_ui(UIS
    src/mainwidget/mainwidget.ui
    src/settingswidget/settingswidget.ui
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
add_subdirectory(plugins/applications)
#add_subdirectory(plugins/chromebookmarks)
#add_subdirectory(plugins/websearch)
#add_subdirectory(plugins/PythonExtensions)
add_subdirectory(plugins/templateExtension)
