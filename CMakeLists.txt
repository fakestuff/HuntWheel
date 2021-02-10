cmake_minimum_required(VERSION 3.19.0)
message(STATUS "Attempting auto locate Vulkan using CMake......")
	
# Find Vulkan Path using CMake's Vulkan Module
# This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
find_package(Vulkan)
if(NOT Vulkan_FOUND)
set(Vulkan_INCLUDE_DIRS "C:/VulkanSDK/1.2.154.1")
endif()
# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()
        
if(NOT Vulkan_FOUND)
    # CMake may fail to locate the libraries but could be able to 
    # provide some path in Vulkan SDK include directory variable
    # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
    message(STATUS "Failed to locate Vulkan SDK, retrying again...")
    if(EXISTS "${VULKAN_PATH}")
        message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
    else()
        message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
        message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
        return()
    endif()
endif()



project(HuntWheel VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3.2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# vulkan-1 library for build Vulkan application.
set(VULKAN_LIB_LIST "vulkan-1")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Include Vulkan header files from Vulkan SDK
	include_directories(AFTER ${VULKAN_PATH}/Include)

	# Link directory for vulkan-1
	link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
endif()


# Define directories and the contained folder and files inside.
if(WIN32)
    source_group("include" REGULAR_EXPRESSION "include/*")
    source_group("source" REGULAR_EXPRESSION "src/*")
endif(WIN32)



# Define include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/)
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/include/")

LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/lib/")
# Gather list of header and source files for compilation
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)
file(GLOB_RECURSE EXTERNAL_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR} REGULAR_EXPRESSION "external/*.cpp")
file(GLOB_RECURSE EXTERNAL_HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR} REGULAR_EXPRESSION "external/*.*")




#add_executable(HuntWheel main.cpp ${CPP_FILES} ${HPP_FILES})
add_executable(HuntWheel mainFromScratch.cpp ${CPP_FILES} ${HPP_FILES} ${EXTERNAL_HPP_FILES} ${EXTERNAL_CPP_FILES})
if(MSVC)
    #add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
	# 自定义编译规则
	# set_property(TARGET HuntWheel APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
endif()

target_link_libraries(HuntWheel PRIVATE glfw imgui::imgui ${VULKAN_LIB_LIST})