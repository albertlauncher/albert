find_package(Qt5Network 5.2 REQUIRED)
find_package(Qt5Sql 5.2 REQUIRED)
find_package(Qt5Svg 5.2 REQUIRED)
find_package(Qt5Widgets 5.2 REQUIRED)

FILE(GLOB_RECURSE SRC src/* metadata.json)

set(LIB
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    albertcore
    globalshortcut
    xdg
)

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
target_link_libraries(${PROJECT_NAME} ${LIB})

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

