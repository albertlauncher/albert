cmake_minimum_required(VERSION 2.8.12)

PROJECT(albert)
SET(TARGET_VERSION_MAJOR 0)
SET(TARGET_VERSION_MINOR 6)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

IF (NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Albert build type is debug")
    SET(CMAKE_BUILD_TYPE "debug" )
ELSE  (NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Albert build type is " ${CMAKE_BUILD_TYPE} )
ENDIF (NOT CMAKE_BUILD_TYPE)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)

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

	# Add platformspecific stuff
    find_package(Qt5X11Extras REQUIRED)
#	find_library(muparser REQUIRED)
	set(SRCS ${SRCS} src/globalhotkey_x11.cpp)

	# Define targets and linked libs
	add_executable(albert ${SRCS} ${UIS} ${DATA} ${RSRC})
	target_link_libraries(albert Qt5::Widgets Qt5::X11Extras muparser X11)

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
