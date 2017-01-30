cmake_minimum_required(VERSION 2.8.12)

project(albert)

find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Network
    Sql
    Svg
    Widgets
)

FILE(GLOB_RECURSE SRC src/* metadata.json)

qt5_wrap_ui(UI
    src/mainwindow/mainwindow.ui
    src/settingswidget/settingswidget.ui
)

qt5_add_resources(QRC
    resources.qrc
)

# Define the target
add_executable(${PROJECT_NAME} ${SRC} ${UI} ${QRC})

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    albertcore
    globalshortcut
    xdg
)

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PRIVATE
        src/
        src/settingswidget/
        src/mainwindow/
)

# Set the RPATH so that libraries get found
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")

# Install target
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

