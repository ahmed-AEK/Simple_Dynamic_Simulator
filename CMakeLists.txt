cmake_minimum_required(VERSION 3.11)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO

project(dummy_nodeEditor)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_CXX_FLAGS "-W4 -EHsc -WX")
endif (WIN32)
if (UNIX)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
endif (UNIX)

# if (UNIX AND ${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# if( NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE "Release")
# ENDIF()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -g")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()
set(DRAWRECT_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(NLopt REQUIRED)

add_subdirectory(src/SDL_Framework)
add_subdirectory(src/toolgui)
add_subdirectory(external/googletest)
add_subdirectory(src/toolguitest)
add_subdirectory(src/NodeEditorApp)
add_subdirectory(src/optimizer)
add_subdirectory(src/optimizertest)
add_subdirectory(src/optimizer_perf)