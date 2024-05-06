#pragma once

#include "toolgui/toolgui_exports.h"

#include <SDL_Framework/SDL_headers.h>
#include <SDL_Framework/SDLCPP.h>
#include <vector>
#include "MouseInteractable.h"
#include "ImageButton.h"
#include <ranges>
#include <span>

class TOOLGUI_API ToolbarScene: public MI::MouseInteractable<ToolbarScene>
{
public:
    ToolbarScene();
    void Draw(SDL_Renderer* renderer);
    const SDL_Rect& GetRect() const noexcept;
    void SetRect(const SDL_Rect& rect);
    std::span<ImageButton*> GetButtons();
    void InsertButton(ImageButton button, int index);
    void RemoveButton(int index);
protected:
    virtual ToolbarScene* OnGetInteractableAtPoint(const SDL_Point& point);
protected:
    std::vector<ImageButton*> m_vec;
    std::vector<ImageButton> m_owned_vec;
    ImageButton m_new_button;
    ImageButton m_save_button;
};
