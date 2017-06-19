# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(albertcore)
add_subdirectory(xdg)
add_subdirectory(globalshortcut)
