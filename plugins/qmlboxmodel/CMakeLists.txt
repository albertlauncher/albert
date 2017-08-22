cmake_minimum_required(VERSION 2.8.12)

PROJECT(qmlboxmodel)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS
    Widgets
    Qml
    Quick
)

qt5_wrap_ui(UI
    forms/configwidget.ui
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC}  ${UI})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME} PRIVATE src/ forms/)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${Qt5Quick_LIBRARIES}
    albertcore
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)

# Define where to install the themes
install(DIRECTORY "share/" DESTINATION "share/albert/org.albert.frontend.boxmodel.qml")
