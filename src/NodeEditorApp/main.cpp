#include "NodeEditorApp.hpp"
#include "toolgui/NodeMacros.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);

    // set up the application data
    std::unique_ptr<node::NodeEditorApplication> ptr = std::make_unique<node::NodeEditorApplication>();
    if (ptr->Initialize())
    {
        SDL_Log("Application Failed to Initialize!");
        return SDL_APP_FAILURE;
    }

    *appstate = ptr.release();
    SDL_Log("Application started successfully!");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* app = reinterpret_cast<node::NodeEditorApplication*>(appstate);
    app->HandleEvent(*event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* app = reinterpret_cast<node::NodeEditorApplication*>(appstate);

    if (!app->IsRunning())
    {
        return SDL_APP_SUCCESS;
    }

    app->Update();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    UNUSED_PARAM(result);

    auto* app = reinterpret_cast<node::NodeEditorApplication*>(appstate);
    delete app;
    SDL_Log("Application quit successfully!");
}
