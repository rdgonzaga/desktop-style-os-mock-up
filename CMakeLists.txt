cmake_minimum_required(VERSION 3.24)
project(CsopesyOS LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# git clone instead of URL downloads: the WinLibs CMake can't verify GitHub's certificate
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.5.1
    GIT_SHALLOW TRUE)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw imgui)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(CsopesyOS
    src/main.cpp
    src/Config.cpp
    src/apps/FileExplorer.cpp
    src/apps/TaskManager.cpp
    src/apps/Terminal.cpp
    src/core/Apps.cpp
    src/core/Clock.cpp
    src/core/Icons.cpp
    src/core/Texture.cpp
    src/core/Theme.cpp
    src/shell/Desktop.cpp
    src/shell/Taskbar.cpp)
target_include_directories(CsopesyOS PRIVATE src)
target_include_directories(CsopesyOS SYSTEM PRIVATE third_party)
target_link_libraries(CsopesyOS PRIVATE imgui)

add_custom_target(copy_runtime_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:CsopesyOS>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/config.txt $<TARGET_FILE_DIR:CsopesyOS>)
add_dependencies(CsopesyOS copy_runtime_files)

if(MSVC)
    target_compile_options(CsopesyOS PRIVATE /W4 /utf-8)
else()
    target_compile_options(CsopesyOS PRIVATE -Wall -Wextra)
endif()

# otherwise MinGW builds need libstdc++ DLLs next to the exe
if(MINGW)
    target_link_options(CsopesyOS PRIVATE -static)
endif()
