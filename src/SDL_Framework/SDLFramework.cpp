#include "SDL_headers.h"
#include "SDLFramework.hpp"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <iostream>

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

    SDLFramework::SDLFramework(uint32_t flags) 
    : m_SDL_init_done(false), m_SDL_TTF_init_done(false)
    {
        m_SDL_init_done = !SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | flags);
        if(!(m_SDL_init_done))
        {   
            std::cout << SDL_GetError() << '\n';
            throw std::runtime_error("Couln't initialize SDL");
        }
        m_SDL_TTF_init_done = !TTF_Init();
        if(!(m_SDL_TTF_init_done))
        {
            throw std::runtime_error("Couldn't initialize SDL_TTF");
        }
        m_SDL_Image_init_done = IMG_Init(IMG_INIT_PNG);
        if(!(m_SDL_Image_init_done))
        {
            throw std::runtime_error("couldn't initialize SDL_Image");
        }
    }
}