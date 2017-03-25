cmake_minimum_required(VERSION 2.8.12)

project(albert)

# Define the target
add_executable(${PROJECT_NAME} main.cpp)

# Link target to libraries
target_link_libraries(${PROJECT_NAME} albertcore)

# Set the RPATH so that libraries get found
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")

# Install target
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

