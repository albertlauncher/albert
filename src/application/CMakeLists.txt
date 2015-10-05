#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
    hotkey/
    mainwidget/
    interfaces/
    pluginsystem/
    pluginsystem/extension/
    settingswidget/
)

set(SRCS
    main.cpp
    albertapp.cpp

    hotkey/hotkeymanager.cpp
    hotkey/hotkeymanager_p.h

    mainwidget/inputline.cpp
    mainwidget/itemdelegate.cpp
    mainwidget/mainwidget.cpp
    mainwidget/proposallist.cpp
    mainwidget/settingsbutton.h

    pluginsystem/pluginloader.cpp
    pluginsystem/pluginmanager.cpp

    pluginsystem/extension/extensionmanager.cpp
    pluginsystem/extension/query.cpp
    pluginsystem/extension/query_p.hpp

    pluginsystem/extension/iextension.h
    pluginsystem/extension/abstractobjects.h

    settingswidget/grabkeybutton.cpp
    settingswidget/settingswidget.cpp
)

set(LIBS
    ${Qt5Widgets_LIBRARIES}
)

qt5_wrap_ui(UIS
    mainwidget/mainwidget.ui
    settingswidget/settingswidget.ui
)

qt5_add_resources(RSRC
    resources.qrc
)


# Do platform specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Qt5X11Extras 5.1 REQUIRED)
    set(SRCS ${SRCS} hotkey/hotkeymanager_x11.cpp)
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
install(DIRECTORY resources/themes DESTINATION share/${PROJECT_NAME} COMPONENT config)

