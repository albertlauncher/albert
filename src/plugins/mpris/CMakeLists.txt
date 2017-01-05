cmake_minimum_required(VERSION 2.8.11)

PROJECT(mpris)

find_package(Qt5Widgets 5.2 REQUIRED)
find_package(Qt5DBus 5.2 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

FILE(GLOB_RECURSE SRC src/*)

set(LIB
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    albertcore
    xdg
)

qt5_wrap_ui(UI
    forms/configwidget.ui
)

add_library(${PROJECT_NAME} SHARED ${SRC} ${UI})

target_link_libraries(${PROJECT_NAME} ${LIB})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/albert/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
