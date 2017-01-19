cmake_minimum_required(VERSION 2.8.11)

project(albertlauncher)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic") #-Wconversion -Wunused
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DQT_NO_DEBUG_OUTPUT ")

# Color make output
SET(CMAKE_COLOR_MAKEFILE ON)

# Suppress gcc details
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_VERBOSE_MAKEFILE ON)
else(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_VERBOSE_MAKEFILE OFF)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build project
add_subdirectory(src/application/)
add_subdirectory(src/lib/)
add_subdirectory(src/plugins/)

# Do platform specific post target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Install desktop entry
    install(FILES data/albert.desktop DESTINATION share/applications )
    # Install icon
    install(FILES src/application/resources/icons/albert.svg DESTINATION share/icons/hicolor/scalable/apps)
    # Define where to install the themes
    install(DIRECTORY data/themes DESTINATION share/${PROJECT_NAME} COMPONENT config)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
