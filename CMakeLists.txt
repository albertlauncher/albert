#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.12)

PROJECT(albert)

SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set( CMAKE_VERBOSE_MAKEFILE on )

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wconversion -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT ")

find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Svg REQUIRED)

include_directories(
    src/
    src/ui/
    src/common/
)

set(SRCS
    TODO
    src/main.cpp
    src/settings.h
    src/singleton.h
#    src/fstreenode.cpp
    src/extensionhandler.cpp
    src/pluginhandler.cpp
    src/globalhotkey.cpp
    src/globalhotkey_p.h
    src/history.cpp
    src/ui/mainwidget.cpp
    src/ui/proposallistview.cpp
    src/ui/inputline.cpp
    src/ui/grabkeybutton.cpp
    src/ui/settingsbutton.cpp
    src/ui/settingsdialog.cpp
    src/common/query.cpp
    src/common/extensioninterface.h
)

set(LIBS
    ${Qt5Widgets_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    Qt5::X11Extras
    #muparser
    X11
)

#set(data
#	data/icons/youtube.svg
#	data/icons/google.svg
#	data/icons/ebay.svg
#	data/icons/amazon.svg
#)

qt5_wrap_ui(UIS
    src/ui/settingsdialog.ui
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # BUILD
    #find_library(muparser REQUIRED)
    find_package(Qt5X11Extras REQUIRED)
    set(SRCS ${SRCS} src/globalhotkey_x11.cpp)
    add_executable(${PROJECT_NAME} ${SRCS} ${UIS} ${DATA} ${RSRC})
    target_link_libraries(${PROJECT_NAME} ${LIBS})

    # INSTALL
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(DIRECTORY src/resources/icons DESTINATION share/${PROJECT_NAME} COMPONENT config)
    install(DIRECTORY src/resources/themes DESTINATION share/${PROJECT_NAME} COMPONENT config)
    install(FILES src/resources/albert.desktop DESTINATION share/applications COMPONENT desktop)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_library(muparser STATIC IMPORTED)
    set_property(TARGET muparser PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/muparser/lib/libmuparser.a")

    # Add platformspecific stuff
    set(SRCS ${SRCS} src/util/hotkey/globalhotkey_win.cpp)

    #Use unicode
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")

    # Define targets and linked libs
    add_executable(${PROJECT_NAME} ${SRCS} ${UIS} ${DATA} ${RSRC})
    target_link_libraries(${PROJECT_NAME} Qt5::Widgets muparser)

    # Define make install target
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_subdirectory(plugins/chromebookmarks)
add_subdirectory(plugins/applauncher)
#add_subdirectory(plugins/websearch)
