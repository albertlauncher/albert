cmake_minimum_required(VERSION 2.8.12)

PROJECT(virtualbox)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.2.0 REQUIRED COMPONENTS
    Widgets
    Xml
)

qt5_add_resources(QRC
    ${PROJECT_NAME}.qrc
)

# Define the target
add_library(${PROJECT_NAME} SHARED ${SRC} ${QRC})

add_definitions(-DRT_OS_LINUX)
add_definitions(-DVBOX_WITH_XPCOM_NAMESPACE_CLEANUP)
set(PATH_XPCOM /usr/lib/virtualbox/sdk/bindings/xpcom)
set(PATH_BIN /usr/lib/virtualbox)

if(NOT EXISTS ${PATH_XPCOM}/include/VirtualBox_XPCOM.h)
    message("${PATH_XPCOM}/include/VirtualBox_XPCOM.h not found. Build will fail!")
endif()


# Set INCLUDE_DIRS and INTERFACE_INCLUDE_DIRS
target_include_directories(${PROJECT_NAME}
    PRIVATE
        src/
        ${PATH_XPCOM}/include
        ${PATH_XPCOM}/include/nsprpub
        ${PATH_XPCOM}/include/string
        ${PATH_XPCOM}/include/xpcom
        ${PATH_XPCOM}/include/ipcd
)

# Link target to libraries
target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    albertcore
    ${PATH_BIN}/VBoxXPCOM.so
    xdg
)

# Install target
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)
