cmake_minimum_required(VERSION 2.8.11)

PROJECT(terminal)

find_package(Qt5Widgets 5.2 REQUIRED)

FILE(GLOB_RECURSE SRC src/*)

set(LIB
    ${Qt5Widgets_LIBRARIES}
    albertcore
    xdg
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

qt5_wrap_ui(UI
    configwidget.ui
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${QRC} ${UI})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} ${LIB})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/albert/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
