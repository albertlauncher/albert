cmake_minimum_required(VERSION 2.8.12)

project(albertcore)

add_definitions(-DCORE) #export branch

# Get Qt libraries
find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Concurrent
    Sql
    Widgets
)

# List files in the source directory
FILE(GLOB_RECURSE SRC include/* src/*)

# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC include/
    PRIVATE src/
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Qt5Concurrent_LIBRARIES}
        ${Qt5Sql_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        xdg
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert)
