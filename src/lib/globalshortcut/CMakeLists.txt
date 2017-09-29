cmake_minimum_required(VERSION 3.1.3)

project(globalshortcut)

add_definitions(-DGLOBALSHORTCUT) #export branch

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Gui)

set(SRC
    include/globalshortcut/globalshortcut_globals.h
    include/globalshortcut/hotkeymanager.h
    src/hotkeymanager.cpp
)

set(LIB ${Qt5Gui_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Qt5 5.5.0 REQUIRED X11Extras)
    set(SRC ${SRC} src/hotkeymanager_x11.cpp)
    set(LIB ${LIB} ${Qt5X11Extras_LIBRARIES} X11)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SRC ${SRC} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
endif()

add_library(${PROJECT_NAME} SHARED ${SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include/
    PRIVATE
        include/globalshortcut/
        src/
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert)
