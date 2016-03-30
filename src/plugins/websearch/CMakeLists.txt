cmake_minimum_required(VERSION 2.8.11)

PROJECT(websearch)

find_package(Qt5Widgets 5.2 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

set(SRC
    metadata.json
    extension.cpp
    configwidget.cpp
    searchengine.cpp
)

qt5_wrap_ui(UI
    configwidget.ui
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

add_library(${PROJECT_NAME} SHARED ${SRC} ${UI} ${QRC})

target_link_libraries(${PROJECT_NAME} ${Qt5Widgets_LIBRARIES} iconlookup)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/${PROJECT_NAME}/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
