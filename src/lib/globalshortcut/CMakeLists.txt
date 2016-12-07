cmake_minimum_required(VERSION 2.8.11)

project(globalshortcut)

# Get Qt libraries
find_package(Qt5Gui 5.2 REQUIRED)

set(SRC
    include/hotkeymanager.h
    src/hotkeymanager.cpp
    src/hotkeymanager_p.h
)

set(LIB
    ${Qt5Gui_LIBRARIES}
)

# Do platform specific pre target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    find_package(Qt5X11Extras 5.2 REQUIRED)

    set(SRC ${SRC} src/hotkeymanager_x11.cpp)
    set(LIB ${LIB} ${Qt5X11Extras_LIBRARIES} X11)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    set(SRC ${SRC} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")

endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Define the target
add_library(${PROJECT_NAME} STATIC ${SRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PRIVATE src
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} ${LIB})
