cmake_minimum_required(VERSION 3.20)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO
option(BUILD_STATIC "Make a static build" OFF)
option(EMSCRIPTEN "EMSCRIPTEN" OFF)
option(FILESYSTEM_SUPPORTED "denotes whether there's a readable and writeable filesystem" ON)

cmake_policy(SET CMP0091 NEW)

if (${CMAKE_VERSION} VERSION_GREATER "3.29")
cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)  # for cppcheck
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

project(DynamicSim VERSION 0.5.1)

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -W4 -EHsc -WX /GR- ")
else(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Wpedantic -Werror -fno-rtti ")
endif (MSVC)

if (UNIX AND BUILD_STATIC AND NOT EMSCRIPTEN)
    string(APPEND CMAKE_CXX_FLAGS " -static-libgcc -static-libstdc++ ")
endif ()

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -sALLOW_MEMORY_GROWTH -Wno-pthreads-mem-growth ")
    set(FILESYSTEM_SUPPORTED FALSE)
endif (EMSCRIPTEN)

if (EMSCRIPTEN AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -sASSERTIONS ")
endif ()

if (FILESYSTEM_SUPPORTED)
    set(FILESYSTEM_SUPPORTED_FLAG "1")
else()
    set(FILESYSTEM_SUPPORTED_FLAG "0")
endif()

# if (UNIX AND ${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# endif()

# set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# if( NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE "Release")
# ENDIF()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -g")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (NOT EMSCRIPTEN)
    enable_testing()
endif (NOT EMSCRIPTEN)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries (DLLs).")

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(NLopt CONFIG REQUIRED)
find_package(lunasvg CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS container_hash numeric_odeint charconv)
find_package(lua CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

add_subdirectory(src/AssetsManager)
add_subdirectory(src/SDL_Framework)
add_subdirectory(src/toolgui)
add_subdirectory(src/NodeModels)
add_subdirectory(src/NetUtils)
add_subdirectory(src/BuiltinClasses)
add_subdirectory(src/NodeSDLStylers)
add_subdirectory(src/SceneLoader)
add_subdirectory(src/GraphicsScene)
add_subdirectory(src/BlockPalette)
add_subdirectory(src/NodeEditorApp)
add_subdirectory(src/optimizer)
add_subdirectory(src/PluginAPI)
add_subdirectory(src/LuaPlugin)

if (NOT EMSCRIPTEN)
    add_subdirectory(src/NetUtilstest)
    add_subdirectory(src/NodeModelstest)
    add_subdirectory(src/SceneLoadertest)
    add_subdirectory(src/toolguitest)
    add_subdirectory(src/optimizertest)
    add_subdirectory(src/LuaPlugintest)
    add_subdirectory(src/optimizer_perf)
endif(NOT EMSCRIPTEN)
