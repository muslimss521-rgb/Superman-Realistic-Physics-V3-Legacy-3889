cmake_minimum_required(VERSION 3.10)
project(SupermanHomelanderMod)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
    src/main.cpp
    src/superman.cpp
    src/camera.cpp
    src/effects.cpp
)

include_directories(include)
add_library(superman SHARED ${SOURCE_FILES})
# Укажи путь к ScriptHookV.lib в своей системе
target_link_libraries(superman PRIVATE "${CMAKE_SOURCE_DIR}/libs/ScriptHookV.lib")
set_target_properties(superman PROPERTIES PREFIX "" OUTPUT_NAME "superman" SUFFIX ".asi")
