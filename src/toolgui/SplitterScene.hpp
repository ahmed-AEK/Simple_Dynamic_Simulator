#pragma once

#include "toolgui/toolgui_exports.h"
#include <SDL_Framework/SDL_headers.h>
#include <SDL_Framework/SDLRenderer.h>
#include <array>
#include "ToolbarScene.h"
#include "MouseInteractable.h"
#include "DrawScene.h"

class TOOLGUI_API SplitterScene: public MI::MouseInteractable<SplitterScene>
{
public:
    void SetRect(const SDL_Rect& rect);
    void Draw(SDL_Renderer* renderer);
    void SetSashPosition(int position);
    const SDL_Rect& GetRect() const noexcept;
    SDL_Rect GetSashRect() const noexcept;
    void DragSash(const SDL_Point& p);
protected:
    virtual SplitterScene* OnGetInteractableAtPoint(const SDL_Point& point) override;
    virtual void OnMouseMove(const SDL_Point& current_mouse_point) override;
    virtual void OnMouseIn() override;
    virtual void OnMouseOut() override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
private:
    ToolbarScene m_toolbar;
    DrawScene m_scene;
    int SashPosition{100};
    int SashWidth{3};
    SDL_Color m_color{255, 0, 0, 255};
    SDLCursor m_resize_cursor;
    bool m_dragging = false;
};