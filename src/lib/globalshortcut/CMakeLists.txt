cmake_minimum_required(VERSION 2.8.12)

project(globalshortcut)

add_definitions(-DGLOBALSHORTCUT) #export branch

# Get Qt libraries
find_package(Qt5 5.2.0 REQUIRED COMPONENTS Gui)

# List files in the source directory
set(SRC
    include/globalshortcut/globalshortcut_globals.h
    include/globalshortcut/hotkeymanager.h
    src/hotkeymanager.cpp
)

set(LIB
    ${Qt5Gui_LIBRARIES}
)

# Do platform specific pre target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    find_package(Qt5 5.2.0 REQUIRED X11Extras)

    set(SRC ${SRC} src/hotkeymanager_x11.cpp)
    set(LIB ${LIB} ${Qt5X11Extras_LIBRARIES} X11)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    set(SRC ${SRC} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")

endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC})

# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include/
    PRIVATE
        include/globalshortcut/
        src/
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert)
