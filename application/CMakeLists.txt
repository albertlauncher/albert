cmake_minimum_required(VERSION 3.1.3)

project(albert)

# Define the target
add_executable(${PROJECT_NAME} main.cpp)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} PRIVATE albertcore)

# Set the RPATH for the library lookup
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/albert")

# Install target
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

