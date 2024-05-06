// provide a portable way to include SDL headers.

#ifdef __linux__
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_ttf.h>
#elif _WIN32
#pragma warning(push)
#pragma warning(disable : 26819)
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
#pragma warning(pop)
#else
    #error "OS missing SDL2 header file includes"
#endif