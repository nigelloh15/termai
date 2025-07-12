cmake_minimum_required(VERSION 4.0.3)
project(terminalai)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/imgui)

set(IMGUI_SOURCES
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_demo.cpp

    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE 
    src/main.cpp
    src/glad.c
    ${IMGUI_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
)

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()
