cmake_minimum_required(VERSION 2.8.11)

project(albertcore)
add_definitions(-DCORE) #export branch

# Get Qt libraries
find_package(Qt5Core 5.2 REQUIRED)
find_package(Qt5Widgets REQUIRED)

# List files in the source directory
FILE(GLOB_RECURSE SRC include/* src/*)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC include/
    PRIVATE src/
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} xdg)
