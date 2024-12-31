cmake_minimum_required(VERSION 3.0)
project(MiniEngine)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES
    main.cpp
    Shader.cpp
    Camera.cpp
    Mesh.cpp
    Model.cpp
    Light.cpp
    imgui.cpp
    imgui_draw.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
)

# Include directories
include_directories(
    include
    path_to_glad/include
    path_to_glfw/include
    path_to_glm
    path_to_assimp/include
    path_to_imgui
    path_to_stb_image
    path_to_nlohmann_json
)

# Link libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(ASSIMP REQUIRED)

add_executable(MiniEngine ${SOURCES})

target_link_libraries(MiniEngine 
    OpenGL::GL 
    glfw 
    assimp
)
