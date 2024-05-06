#pragma once

#include "toolgui/toolgui_exports.h"

#include "MouseInteractable.h"
#include <vector>

struct TOOLGUI_API Square
{
    SDL_Rect rect;
};

class TOOLGUI_API DrawScene: public MI::MouseInteractable<DrawScene>
{
public:
    DrawScene();
    void SetRect(const SDL_Rect &rect);
    void Draw(SDL_Renderer* renderer);
    void AddSquare(int left, int top, int width);
    SDL_Point ScreenToSceneCoords(const SDL_Point& point);
protected:
    virtual void OnMouseMove(const SDL_Point& current_mouse_point);
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
private:
    virtual DrawScene* OnGetInteractableAtPoint(const SDL_Point& point);
    std::vector<Square> m_shapes;
    int m_current_dragged_shape{};
    bool b_is_dragging{};
    SDL_Point m_shape_drag_start_point{};
};


