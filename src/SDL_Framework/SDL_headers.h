// provide a portable way to include SDL headers.

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 26819)
#endif

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#ifdef _WIN32
#pragma warning(pop)
#endif
