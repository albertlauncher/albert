#http://doc.qt.io/qt-5/cmake-manual.html
cmake_minimum_required(VERSION 2.8.12)

PROJECT(albert)

SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra")

IF (NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Set build type explicitely to debug")
    SET(CMAKE_BUILD_TYPE "debug" )
ENDIF (NOT CMAKE_BUILD_TYPE)

find_package(Qt5Widgets 5.1 REQUIRED)
MESSAGE(STATUS "Qt5Widgets_VERSION " ${Qt5Widgets_VERSION_STRING})

find_package(Qt5Svg REQUIRED)
MESSAGE(STATUS "Qt5Svg_VERSION " ${Qt5Svg_VERSION})

include_directories(
	src/
    src/common/
)

set(SRCS
	src/main.cpp
	src/settings.cpp
	src/query.cpp
	src/extensionhandler.cpp
    src/pluginhandler.cpp
    src/globalhotkey.cpp
    src/globalhotkey_p.h
	src/mainwidget.cpp
	src/proposallistview.cpp
	src/inputline.cpp
	src/history.cpp
    src/grabkeybutton.cpp
	src/settingsbutton.cpp
	src/settingsdialog.cpp
    src/common/extensioninterface.h
)

#set(data
#	data/icons/youtube.svg
#	data/icons/google.svg
#	data/icons/ebay.svg
#	data/icons/amazon.svg
#)

qt5_wrap_ui(UIS
	src/settingsdialog.ui
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


#linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    find_package(Qt5X11Extras REQUIRED)
    MESSAGE(STATUS "Qt5X11Extras_VERSION " ${Qt5X11Extras_VERSION})

#	find_library(muparser REQUIRED)


	set(SRCS ${SRCS} src/globalhotkey_x11.cpp)

	# Define targets and linked libs
	add_executable(albert ${SRCS} ${UIS} ${DATA} ${RSRC})

    target_link_libraries(albert
        ${Qt5Widgets_LIBRARIES}
        ${Qt5X11Extras_LIBRARIES}
        Qt5::X11Extras
#        muparser
        X11
)

	# Define make install target
	install(TARGETS albert RUNTIME DESTINATION bin)
	install(DIRECTORY src/resources/icons DESTINATION share/albert COMPONENT config)
	install(DIRECTORY src/resources/themes DESTINATION share/albert COMPONENT config)
	install(FILES src/resources/albert.desktop DESTINATION share/applications COMPONENT desktop)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#win
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	add_library(muparser STATIC IMPORTED)
	set_property(TARGET muparser PROPERTY
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/muparser/lib/libmuparser.a")

	# Add platformspecific stuff
	set(SRCS ${SRCS} src/util/hotkey/globalhotkey_win.cpp)

	#Use unicode
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")

	# Define targets and linked libs
	add_executable(albert ${SRCS} ${UIS} ${DATA} ${RSRC})
	target_link_libraries(albert Qt5::Widgets muparser)

	# Define make install target
	install(TARGETS albert RUNTIME DESTINATION bin)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")



add_subdirectory(plugins/applauncher)
