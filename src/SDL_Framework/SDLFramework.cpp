#include "SDL_headers.h"
#include "SDLFramework.hpp"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

namespace SDL
{
    SDLFramework::~SDLFramework()
    {
        if (m_SDL_Image_init_done)
        {
            IMG_Quit();
        }
        if (m_SDL_TTF_init_done)
        {
            TTF_Quit();
        }
        if (m_SDL_init_done)
        {
            SDL_Quit();
        }
    }

    SDLFramework::SDLFramework() 
    : m_SDL_init_done(false), m_SDL_TTF_init_done(false), m_SDL_Image_init_done(false)
    {
    }

    bool SDLFramework::Init(uint32_t flags)
    {
        m_SDL_init_done = !SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | flags);
        if (!(m_SDL_init_done))
        {
            SDL_Log("%s",SDL_GetError());
            SDL_Log("Couln't initialize SDL");
            return false;
        }

        m_SDL_TTF_init_done = !TTF_Init();
        if (!(m_SDL_TTF_init_done))
        {
            SDL_Log("Couldn't initialize SDL_TTF");
            return false;
        }

        m_SDL_Image_init_done = IMG_Init(IMG_INIT_PNG);
        if (!(m_SDL_Image_init_done))
        {
            SDL_Log("couldn't initialize SDL_Image");
            return false;
        }

        return true;
    }
}