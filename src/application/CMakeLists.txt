find_package(Qt5Widgets 5.1 REQUIRED)

include_directories(
    hotkey/
    mainwindow/
    interfaces/
    pluginsystem/
    settingswidget/
)

set(SRC
    main.cpp
    albertapp.cpp
    hotkey/hotkeymanager.cpp
    hotkey/hotkeymanager_p.h
    mainwindow/mainwindow.cpp
    mainwindow/inputline.cpp
    mainwindow/itemdelegate.cpp
    mainwindow/proposallist.cpp
    mainwindow/settingsbutton.h
    pluginsystem/pluginloader.cpp
    pluginsystem/pluginmanager.cpp
    pluginsystem/extensionmanager.cpp
    pluginsystem/query.cpp
    pluginsystem/query_p.hpp
    pluginsystem/iextension.h
    pluginsystem/abstractobjects.hpp
    pluginsystem/objects.hpp
    settingswidget/grabkeybutton.cpp
    settingswidget/settingswidget.cpp
    settingswidget/pluginmodel.cpp
)

set(LIB
    ${Qt5Widgets_LIBRARIES}
)

qt5_wrap_ui(UI
    mainwindow/mainwindow.ui
    settingswidget/settingswidget.ui
)

qt5_add_resources(QRC
    resources.qrc
)


# Do platform specific pre target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Make sure x11 module is present
    find_package(Qt5X11Extras 5.1 REQUIRED)
    # Platform switches
    set(SRC ${SRC} hotkey/hotkeymanager_x11.cpp)
    set(LIB ${LIB} ${Qt5X11Extras_LIBRARIES} X11)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #TODO INSTALL
    set(SRCS ${SRCS} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")


# Define target
add_executable(${PROJECT_NAME} ${SRC} ${UI} ${QRC})
target_link_libraries(${PROJECT_NAME} ${LIB})


# Do platform specific post target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Define where to install the binary
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
