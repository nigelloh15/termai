cmake_minimum_required(VERSION 4.0.3)
project(terminalai)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Python3 3.12.1 EXACT COMPONENTS Interpreter Development)

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
    src/shell/shell.cpp
    ${IMGUI_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)

target_include_directories(${PROJECT_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Python3_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
)

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

# Path to virtual environment inside build folder
set(VENV_DIR "${CMAKE_BINARY_DIR}/venv")

# Create Python venv and install openai package
add_custom_command(
    OUTPUT "${VENV_DIR}/bin/activate"
    COMMAND python3 -m venv "${VENV_DIR}"
    COMMAND "${VENV_DIR}/bin/pip" install --upgrade pip
    COMMAND "${VENV_DIR}/bin/pip" install openai
    COMMENT "Creating Python virtual environment and installing openai"
)

add_custom_target(
    setup_python_env ALL
    DEPENDS "${VENV_DIR}/bin/activate"
)

add_dependencies(${PROJECT_NAME} setup_python_env)

# --- Copy Python files to build directory ---

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/python
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/python
)

# --- Copy venv to build directory ---

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${VENV_DIR}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/venv
)

# --- Copy Python files and venv into the macOS .app bundle's Resources folder ---

if(APPLE)
    set(APP_RESOURCES_DIR
        $<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/python
        ${APP_RESOURCES_DIR}/python
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${VENV_DIR}
        ${APP_RESOURCES_DIR}/venv
    )
endif()
