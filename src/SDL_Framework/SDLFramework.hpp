#pragma once

#include <cstdint>
#include "toolgui/NodeMacros.h"

namespace SDL
{
    class SDLFramework
    {
    public:
        SDLFramework(SDLFramework&) = delete;
        SDLFramework& operator=(SDLFramework&) = delete;
        SDLFramework();
        bool Init(uint32_t flags = 0);
        ~SDLFramework();
    private:
        bool m_SDL_init_done;
        bool m_SDL_TTF_init_done;
    };
}