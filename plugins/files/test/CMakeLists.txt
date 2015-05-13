cmake_minimum_required(VERSION 2.8.11)
project(files_test)
enable_testing()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

set(${PROJECT_NAME}_SRCS
    fileindextest.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
add_test(${PROJECT_NAME} ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Qt5::Test)
