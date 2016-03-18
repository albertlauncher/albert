set(LIB_NAME websearch)

find_package(Qt5Widgets 5.1 REQUIRED)

include_directories(
    ../../application/
    ../../shared
    ../../application/pluginsystem/extension
)

set(SRC
    metadata.json
    extension.cpp
    configwidget.cpp
    searchengine.cpp
    ../../shared/iconlookup/xdgiconlookup.cpp
)

qt5_wrap_ui(UI
    configwidget.ui
)

qt5_add_resources(QRC
    ${LIB_NAME}.qrc
)

add_library(${LIB_NAME} SHARED ${SRC} ${UI} ${QRC})
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${LIB_NAME} ${Qt5Widgets_LIBRARIES} )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Define where to install the lib
    install(TARGETS ${LIB_NAME} LIBRARY DESTINATION share/${PROJECT_NAME}/plugins)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
