include("$ENV{VCPKG_ROOT}/triplets/community/wasm32-emscripten.cmake")

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/wasm_platform.cmake")
