cmake_minimum_required(VERSION 2.8.12)

project(albertcore)

add_definitions(-DCORE) #export branch

# Get Qt libraries
find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Concurrent
    Network
    Sql
    Svg
    Widgets
)

# List files in the source directory
FILE(GLOB_RECURSE SRC include/* src/*)

qt5_wrap_ui(UI
    src/albert/mainwindow/mainwindow.ui
    src/albert/settingswidget/settingswidget.ui
)

qt5_add_resources(QRC
    resources.qrc
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI} ${QRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include/
    PRIVATE
        src/
        src/albert/
        src/albert/settingswidget/
        src/albert/mainwindow/
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Qt5Concurrent_LIBRARIES}
        ${Qt5Network_LIBRARIES}
        ${Qt5Sql_LIBRARIES}
        ${Qt5Svg_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        globalshortcut
        xdg
)

# Set the RPATH for the library lookup
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert)
