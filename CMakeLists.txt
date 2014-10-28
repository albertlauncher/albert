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
find_package(Qt5X11Extras REQUIRED)
find_library(muparser REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
	src/
	src/ui/
	src/services/
)

set(SRCS
	src/main.cpp
	src/engine.cpp
	src/globalhotkey.cpp
	src/services/fileindex/fileindex.cpp
	src/services/bookmarkindex/bookmarkindex.cpp
	src/services/calculator/calculator.cpp
	src/services/websearch/websearch.cpp
	src/services/fileindex/fileitem.cpp
	src/services/calculator/calculatoritem.cpp
	src/services/bookmarkindex/bookmarkitem.cpp
	src/services/websearch/websearchitem.cpp
	src/services/appindex/appindexwidget.cpp
	src/services/appindex/appindex.cpp
	src/services/appindex/appitem.cpp
	src/services/bookmarkindex/bookmarkindexwidget.cpp
	src/services/calculator/calculatorwidget.cpp
	src/services/fileindex/fileindexwidget.cpp
	src/services/websearch/websearchwidget.cpp
	src/services/indexservice.cpp
	src/ui/mainwidget.cpp
	src/ui/proposallistdelegate.cpp
	src/ui/proposallistview.cpp
	src/ui/inputline.cpp
	src/ui/settingsbutton.cpp
	src/ui/settingsdialog.cpp
	src/ui/hotkeywidget.cpp
)
set(data
	data/albert.conf
	data/icons/youtube.svg
	data/icons/google.svg
	data/icons/ebay.svg
	data/icons/amamzon.conf
)

qt5_wrap_ui(UIS
	src/ui/settingsdialog.ui
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

	# Add platformspecific sources
	set(SRCS ${SRCS} src/globalhotkey_x11.cpp)

	# Define targets and linked libs
	add_executable(albert ${SRCS} ${UIS} ${DATA} ${RSRC})
	target_link_libraries(albert Qt5::Widgets Qt5::X11Extras muparser X11)

	# Define make install target
	install(TARGETS albert RUNTIME DESTINATION bin)
	install(DIRECTORY data/icons DESTINATION share/albert COMPONENT config)
	install(DIRECTORY data/themes DESTINATION share/albert COMPONENT config)
	install(FILES data/icon.svg DESTINATION share/albert COMPONENT icon)
	install(FILES data/albert.desktop DESTINATION share/applications COMPONENT desktop)
	install(FILES data/albert.conf DESTINATION /etc/xdg/albert COMPONENT config)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#win
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(SRCS ${SRCS} src/globalhotkey_win.cpp)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
