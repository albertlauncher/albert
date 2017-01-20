cmake_minimum_required(VERSION 2.8.11)

project(xdg)
add_definitions(-DXDG) #export branch

# Get Qt libraries
find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Core
    Gui
)

# List files in the source directory
FILE(GLOB SRC src/* include/*)

# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC include/
    PRIVATE src/
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Qt5Core_LIBRARIES}
        ${Qt5Gui_LIBRARIES}
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert)
