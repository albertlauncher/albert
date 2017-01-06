cmake_minimum_required(VERSION 2.8.11)

PROJECT(debug)

find_package(Qt5Widgets 5.2 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

FILE(GLOB_RECURSE SRC src/* metadata.json)

set(LIB
    ${Qt5Widgets_LIBRARIES}
    albertcore
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

qt5_wrap_ui(UI configwidget.ui)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI} ${QRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} ${LIB})

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION share/albert/plugins)
    endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(${CMAKE_BUILD_TYPE} MATCHES "Debug")

