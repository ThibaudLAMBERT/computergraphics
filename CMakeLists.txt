cmake_minimum_required(VERSION 3.0$1)
project(finalProject3)

find_package(OpenGL REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glad-opengl-3.3/include/
        external/
        finalProject3/
)

add_executable(main
        finalProject3/main.cpp
        finalProject3/render/shader.cpp
)
target_link_libraries(main
        ${OPENGL_LIBRARY}
        glfw
        glad
)

