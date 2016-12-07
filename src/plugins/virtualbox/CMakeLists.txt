cmake_minimum_required(VERSION 2.8.11)

PROJECT(virtualbox)

find_package(Qt5Widgets 5.2 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

set(SRC
    metadata.json
    extension.cpp
)

set(LIB
    ${Qt5Widgets_LIBRARIES}
    xdg
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

add_library(${PROJECT_NAME} SHARED ${SRC} ${QRC} )

target_link_libraries(${PROJECT_NAME} ${LIB})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/albert/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
