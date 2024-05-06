#pragma once
#include "toolgui/toolgui_exports.h"

#include "MouseInteractable.h"
#include <SDL_Framework/SDLCPP.h>
#include <string_view>
#include <string>
#include <functional>

class TOOLGUI_API ImageButton: public MI::MouseInteractable<ImageButton>
{
public:
    ImageButton(const std::string_view img_path, std::function<void(void)>&& fn = std::function<void(void)>());
    void Draw(SDL_Renderer* renderer);
    const SDL_Rect& GetRect() const noexcept;
    void SetRect(const SDL_Rect& rect);
    void loadimage();
    void Bind(std::function<void(void)>&& fn);
    const std::string_view GetPath() const;
protected:
    virtual ImageButton* OnGetInteractableAtPoint(const SDL_Point& point) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
    virtual void OnMouseOut() override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;

private:
    std::string m_source_path;
    std::function<void(void)> m_OnClick;
    SDLSurface m_btn_surface;
    SDLTexture m_btn_tex;
    SDL_Rect m_rect{ 0,0,0,0 };
    bool m_texture_loaded = false;
    bool m_being_clicked = false;
};

