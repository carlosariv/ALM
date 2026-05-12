cmake_minimum_required(VERSION 3.15)

project(ALM VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# if (MSVC)
#     add_compile_options(/we4062)
# endif()

find_package(LLVM)

add_subdirectory(src)

set_property(TARGET ALM PROPERTY RUNTIME_OUTPUT_DIRECTORY        "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set_property(TARGET ALM PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set_property(TARGET ALM PROPERTY VS_DEBUGGER_WORKING_DIRECTORY   "${CMAKE_CURRENT_SOURCE_DIR}")

# target_link_libraries(ALM)
