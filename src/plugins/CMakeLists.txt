add_subdirectory(applications)
add_subdirectory(calculator)
add_subdirectory(chromebookmarks)
add_subdirectory(files)
add_subdirectory(system)
add_subdirectory(websearch)
add_subdirectory(terminal)

# Non-release plugins
if(${BUILD_DEBUG_EXTENSIONS})
    add_subdirectory(templateExtension)
    add_subdirectory(microsleep)
endif(${BUILD_DEBUG_EXTENSIONS})
