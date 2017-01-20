cmake_minimum_required(VERSION 2.8.11)

PROJECT(virtualbox)

find_package(Qt5Widgets 5.2 REQUIRED)
find_package(Qt5Xml 5.2 REQUIRED)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Widgets
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${QRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    albertcore
    xdg
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)
