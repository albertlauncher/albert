# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(applications)
add_subdirectory(calculator)
add_subdirectory(chromebookmarks)
add_subdirectory(externalextensions)
add_subdirectory(files)
add_subdirectory(system)
add_subdirectory(terminal)
add_subdirectory(virtualbox)
add_subdirectory(websearch)

# Non-release plugins
if(${BUILD_DEBUG_EXTENSIONS})
    add_subdirectory(templateExtension)
    add_subdirectory(debug)
endif(${BUILD_DEBUG_EXTENSIONS})

