// provide a portable way to include SDL headers.

#ifdef __linux__
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_ttf.h>
#elif _WIN32
#pragma warning(push)
#pragma warning(disable : 26819)
    #include <SDL3/SDL.h>
    #include <SDL3_image/SDL_image.h>
    #include <SDL3_ttf/SDL_ttf.h>
#pragma warning(pop)
#else
    #error "OS missing SDL3 header file includes"
#endif