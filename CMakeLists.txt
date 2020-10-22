cmake_minimum_required(VERSION 3.18.0)
project(HuntWheel VERSION 0.1.0)
message([WARNING] "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")


INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/vcpkg/installed/x86-windows/include")
LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/vcpkg/installed/x86-windows/lib")
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 3.3.2 CONFIG REQUIRED)

add_executable(HuntWheel main.cpp)

target_link_libraries(HuntWheel PRIVATE glfw)