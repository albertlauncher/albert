# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Set the RPATH for the library lookup
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")

# FrontendPlugins
add_subdirectory(widgetboxmodel)
add_subdirectory(qmlboxmodel)

# ExtensionPlugins
add_subdirectory(applications)
add_subdirectory(calculator)
add_subdirectory(chromebookmarks)
add_subdirectory(externalextensions)
add_subdirectory(files)
add_subdirectory(firefoxbookmarks)
add_subdirectory(ssh)
add_subdirectory(mpris)
add_subdirectory(system)
add_subdirectory(terminal)
add_subdirectory(virtualbox)
add_subdirectory(websearch)

# Non-release plugins
if(${BUILD_DEBUG_EXTENSIONS})
    add_subdirectory(templateExtension)
    add_subdirectory(debug)
endif(${BUILD_DEBUG_EXTENSIONS})

