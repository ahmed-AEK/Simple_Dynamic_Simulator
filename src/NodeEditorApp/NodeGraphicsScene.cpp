#include "NodeGraphicsScene.hpp"
#include "ExampleContextMenu.hpp"
#include "SDL2/SDL_ttf.h"
#include "GraphicsScene/GraphicsObject.hpp"
#include "toolgui/Scene.hpp"

node::NodeGraphicsScene::NodeGraphicsScene(SDL_Rect rect, node::Scene* parent)
: node::GraphicsScene(rect, parent)
{
    
}

void node::NodeGraphicsScene::Draw(SDL_Renderer* renderer)
{
    DrawDots(renderer);
    node::GraphicsScene::Draw(renderer);
    // DrawCoords(renderer);
    // DrawCurrentInsertMode(renderer);
}

void node::NodeGraphicsScene::DrawDots(SDL_Renderer* renderer) const
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    const int start_x = static_cast<int>(GetSpaceRect().x / m_dotspace) * m_dotspace;
    const int start_y = static_cast<int>(GetSpaceRect().y / m_dotspace) * m_dotspace;
    const SDL_Point start_point_screen = SpaceToScreenPoint({ start_x, start_y });
    SDL_Point step_screen = SpaceToScreenVector({ m_dotspace, m_dotspace });
    int dot_width = 2;
    if (GetZoomScale() > 1.3)
    {
        dot_width = 1;
    }
    else if (GetZoomScale() < 0.7)
    {
        dot_width = 3;
    }
    for (int i = start_point_screen.x; i < GetRect().w; i += step_screen.x)
    {
        for (int j = start_point_screen.y + 5; j < GetRect().h; j += step_screen.y)
        {
            SDL_Rect rect{ i,j,dot_width,dot_width };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
void node::NodeGraphicsScene::DrawCurrentInsertMode(SDL_Renderer* renderer) const
{
    TTFFont font = TTFFont{ TTF_OpenFont("./assets/FreeSans.ttf", 24) };
    SDL_Color Black = { 122, 122, 122, 255 };
    const char* value = GraphicsSceneMode::Insert == GetMode() ? "True" : "False";
    SDLSurface message_surface = SDLSurface{ TTF_RenderText_Solid(font.get(), value, Black) };
    SDLTexture Message = SDLTexture{ SDL_CreateTextureFromSurface(renderer, message_surface.get()) };

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = GetRect().x;  //controls the rect's x coordinate 
    Message_rect.y = GetRect().y + GetRect().h * 4 / 5;; // controls the rect's y coordinte
    Message_rect.w = 200; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    SDL_RenderCopy(renderer, Message.get(), NULL, &Message_rect);
}
void node::NodeGraphicsScene::DrawCoords(SDL_Renderer* renderer) const
{
    TTFFont font = TTFFont{ TTF_OpenFont("./assets/FreeSans.ttf", 24) };
    SDL_Color Black = { 122, 122, 122, 255 };
    std::string point_string = std::to_string(m_current_hover_point.x) + std::string(",") + std::to_string(m_current_hover_point.y) +
        std::string(",,") + std::to_string(m_screen_hover_point.x) + std::string(",") + std::to_string(m_screen_hover_point.y);
    SDLSurface message_surface = SDLSurface{ TTF_RenderText_Solid(font.get(), point_string.c_str(), Black) };
    SDLTexture Message = SDLTexture{ SDL_CreateTextureFromSurface(renderer, message_surface.get()) };

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = GetRect().x + GetRect().w * 2 / 5;  //controls the rect's x coordinate 
    Message_rect.y = GetRect().y + GetRect().h * 4 / 5;; // controls the rect's y coordinte
    Message_rect.w = 400; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    SDL_RenderCopy(renderer, Message.get(), NULL, &Message_rect);
}
MI::ClickEvent node::NodeGraphicsScene::OnRMBUp(const SDL_Point& p)
{
    MI::ClickEvent ret =  GraphicsScene::OnRMBUp(p);
    if (ret != MI::ClickEvent::NONE)
    {
        return ret;
    }
    std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this->p_parent);
    this->p_parent->ShowContextMenu(std::move(menu), {p.x, p.y});
    return MI::ClickEvent::CLICKED;
}

void node::NodeGraphicsScene::OnMouseMove(const SDL_Point& p)
{
    node::GraphicsScene::OnMouseMove(p);
    m_current_hover_point = ScreenToSpacePoint(p);
    m_screen_hover_point = p;
}
