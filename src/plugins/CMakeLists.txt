add_subdirectory(applications)
add_subdirectory(calculator)
add_subdirectory(chromebookmarks)
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
