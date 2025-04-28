string(APPEND CMAKE_CXX_FLAGS_INIT " -fexceptions -pthread ")
string(APPEND CMAKE_C_FLAGS_INIT " -fexceptions -pthread ")

include("$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
