cmake_minimum_required(VERSION 3.1.3)

project(xdg)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Gui)

add_library(${PROJECT_NAME} SHARED
        src/iconlookup.cpp
        src/themefileparser.cpp
)

add_library(albert::xdg ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        include/xdg
        src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE -DXDG_DEF) #export branch

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Gui)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert)
