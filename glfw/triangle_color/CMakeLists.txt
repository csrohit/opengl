cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1 )

project(triangle_color VERSION 0.1)
add_executable(${PROJECT_NAME} main.cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(OPENGL_GL_PREFERENCE GLVND)
if (POLICY CMP0072)
  cmake_policy (SET CMP0072 NEW)
endif(POLICY CMP0072)
# add include path
# target_include_directories(${PROJECT_NAME} PUBLIC
#     "/usr/local/include/"
#     )

if (UNIX)
    
    message(STATUS "Detected UNIX")
    if (APPLE)

        message(STATUS "Detected APPLE")

    else ()
    message(STATUS "Detected linux")

    find_package(glfw3 3.3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} glfw)

    find_package(GLEW REQUIRED)
    target_link_libraries(${PROJECT_NAME} GLEW)

    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)
    
    endif ()
else ()
    message(STATUS "Detected windows")

endif ()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

