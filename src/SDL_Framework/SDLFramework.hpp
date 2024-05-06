#pragma once

#include <cstdint>
#include "toolgui/NodeMacros.h"

namespace SDL
{
    class SDLFramework
    {
    public:
        ~SDLFramework();
        SDLFramework(SDLFramework&) = delete;
        SDLFramework& operator=(SDLFramework&) = delete;
        SDLFramework(uint32_t flags = 0);
    private:
        bool m_SDL_init_done;
        bool m_SDL_TTF_init_done;
        bool m_SDL_Image_init_done;
    };
}