cmake_minimum_required(VERSION 4.0.3)
project(terminalai)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp src/glad.c)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
)

