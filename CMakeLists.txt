cmake_minimum_required(VERSION 3.1.3)  # Least supporting CMAKE_CXX_STANDARD

project(albertlauncher)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic") #-Wconversion -Wunused
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")

# Color make output
SET(CMAKE_COLOR_MAKEFILE ON)

# Suppress gcc details
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_VERBOSE_MAKEFILE ON)
else(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_VERBOSE_MAKEFILE OFF)
endif()

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build project
add_subdirectory(application/)
add_subdirectory(lib/)
add_subdirectory(plugins/)

# Do platform specific post target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # Install desktop entry
    install(FILES data/albert.desktop DESTINATION share/applications )

    # Install icon
    install(FILES src/lib/albertcore/resources/icons/albert.svg DESTINATION share/icons/hicolor/scalable/apps)


endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
