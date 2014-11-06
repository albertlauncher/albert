cmake_minimum_required(VERSION 2.8.11)

if(CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_CXX_FILES "-O2 -g -Wall")
else()
	set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_CXX_FILES "-O0 -g -Wall")
endif()

project(albert)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
	src/
	src/hotkey/
	src/services/
	src/ui/mainwidget/
	src/ui/settingsdialog/
)

set(SRCS
	src/main.cpp
	src/engine.cpp
	src/hotkey/globalhotkey.cpp
	src/services/service.h
	src/services/indexservice.cpp
	src/services/appindex/appindexwidget.cpp
	src/services/appindex/appindex.cpp
	src/services/appindex/appitem.cpp
	src/services/bookmarkindex/bookmarkindex.cpp
	src/services/bookmarkindex/bookmarkitem.cpp
	src/services/bookmarkindex/bookmarkindexwidget.cpp
	src/services/calculator/calculator.cpp
	src/services/calculator/calculatoritem.cpp
	src/services/calculator/calculatorwidget.cpp
	src/services/fileindex/fileindex.cpp
	src/services/fileindex/fileindexwidget.cpp
	src/services/fileindex/fileitem.cpp
	src/services/websearch/websearch.cpp
	src/services/websearch/websearchitem.cpp
	src/services/websearch/websearchwidget.cpp
	src/ui/mainwidget/mainwidget.cpp
	src/ui/mainwidget/proposallistview.cpp
	src/ui/mainwidget/inputline.cpp
	src/ui/mainwidget/settingsbutton.cpp
	src/ui/settingsdialog/settingsdialog.cpp
	src/ui/settingsdialog/hotkeywidget.cpp
)

set(data
	data/icons/youtube.svg
	data/icons/google.svg
	data/icons/ebay.svg
	data/icons/amazon.svg
)

qt5_wrap_ui(UIS
	src/ui/settingsdialog/settingsdialog.ui
	src/services/appindex/appindexwidget.ui
	src/services/bookmarkindex/bookmarkindexwidget.ui
	src/services/calculator/calculatorwidget.ui
	src/services/fileindex/fileindexwidget.ui
	src/services/websearch/websearchwidget.ui
)

qt5_add_resources(RSRC
	src/resources.qrc
)


#linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	# Add platformspecific stuff
	find_package(Qt5X11Extras REQUIRED)
	find_library(muparser REQUIRED)
	set(SRCS ${SRCS} src/hotkey/globalhotkey_x11.cpp)

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

		# Add platformspecific stuff
		set(SRCS ${SRCS} src/globalhotkey_win.cpp)

		#Use unicode
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")

		# Define targets and linked libs
		add_executable(albert ${SRCS} ${UIS} ${DATA} ${RSRC})
		target_link_libraries(albert Qt5::Widgets)

		# Define make install target
		install(TARGETS albert RUNTIME DESTINATION bin)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
