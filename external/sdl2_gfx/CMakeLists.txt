cmake_minimum_required(VERSION 3.11)

project(sdl2_gfx)

find_package(SDL2 REQUIRED)

add_library(sdl2_gfx 
SDL2_framerate.c
SDL2_gfxPrimitives.c
SDL2_imageFilter.c
SDL2_rotozoom.c
SDL2_framerate.h
SDL2_gfxPrimitives.h
SDL2_gfxPrimitives_font.h
SDL2_imageFilter.h
SDL2_rotozoom.h
)

include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/sdl2_gfx
    CACHE PATH "Location of header files" )

target_include_directories(sdl2_gfx PRIVATE .)
target_include_directories(sdl2_gfx
                           PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                            $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(sdl2_gfx PUBLIC SDL2::SDL2)
install(TARGETS sdl2_gfx EXPORT sdl2_gfxTargets DESTINATION lib)
install(FILES 
SDL2_framerate.h
SDL2_gfxPrimitives.h
SDL2_gfxPrimitives_font.h
SDL2_imageFilter.h
SDL2_rotozoom.h
DESTINATION include/sdl2_gfx)

include(CMakePackageConfigHelpers)
configure_package_config_file(sdl2_gfxConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sdl2_gfxConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdl2_gfx)
  
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdl2_gfxConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdl2_gfx )
		
install(EXPORT sdl2_gfxTargets NAMESPACE sdl2:: DESTINATION cmake)