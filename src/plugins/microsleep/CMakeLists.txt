set(LIB_NAME microsleep)

find_package(Qt5Widgets 5.1 REQUIRED)

include_directories(
    ../../application
    ../../application/pluginsystem
)

set(SRC
    metadata.json
    extension.cpp
)

add_library(${LIB_NAME} SHARED ${SRC})
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${LIB_NAME} ${Qt5Widgets_LIBRARIES} )

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        # Define where to install the lib
        install(TARGETS ${LIB_NAME} LIBRARY DESTINATION share/${PROJECT_NAME}/plugins)
    endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(${CMAKE_BUILD_TYPE} MATCHES "Debug")

