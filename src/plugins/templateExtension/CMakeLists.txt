cmake_minimum_required(VERSION 2.8.11)

PROJECT(template)

find_package(Qt5Widgets 5.2 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

set(SRC
    metadata.json
    extension.cpp
    configwidget.cpp
    item.cpp
)

set(LIB
    ${Qt5Widgets_LIBRARIES}
)

qt5_wrap_ui(UI
    configwidget.ui
)

add_library(${PROJECT_NAME} SHARED ${SRC} ${UI})

target_link_libraries(${PROJECT_NAME} ${LIB})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/albert/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
