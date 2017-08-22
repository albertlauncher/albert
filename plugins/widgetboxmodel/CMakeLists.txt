cmake_minimum_required(VERSION 2.8.12)

PROJECT(widgetboxmodel)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Widgets
)

qt5_wrap_ui(UI
    forms/frontend.ui
    forms/configwidget.ui
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME} PRIVATE src/ forms/)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    albertcore
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)

# Define where to install the themes
install(DIRECTORY "share/" DESTINATION "share/albert/org.albert.frontend.boxmodel.widgets")
