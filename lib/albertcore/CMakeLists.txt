cmake_minimum_required(VERSION 3.1.3)

project(albertcore)

add_definitions(-DCORE) #export branch
if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
    add_definitions(-DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}") #export branch
endif()

find_package(Qt5 5.5.0
    COMPONENTS
        Concurrent
        Network
        Sql
        Widgets
    OPTIONAL_COMPONENTS
        Charts
)

file(GLOB_RECURSE SRC include/* src/*)
if (NOT Qt5Charts_FOUND)
    list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/core/settingswidget/statswidget.h")
    list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/core/settingswidget/statswidget.cpp")
endif()

add_library(${PROJECT_NAME} SHARED ${SRC} resources.qrc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include/
    PRIVATE
        include/core
        include/util
        src/
        src/core/
        src/core/settingswidget/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Concurrent
        Qt5::Network
        Qt5::Sql
        Qt5::Widgets
        globalshortcut
        xdg
)
if (Qt5Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Charts)
endif()

# Set the RPATH for the library lookup
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert)



