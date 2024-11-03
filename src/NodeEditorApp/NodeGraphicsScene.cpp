#include "NodeGraphicsScene.hpp"
#include "ExampleContextMenu.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"

node::NodeGraphicsScene::NodeGraphicsScene(SDL_FRect rect, node::Scene* parent)
    : node::GraphicsScene{ rect, parent }
{
    SetFocusable(true);
}

void node::NodeGraphicsScene::Draw(SDL_Renderer* renderer)
{
    DrawDots(renderer);
    node::GraphicsScene::Draw(renderer);
    // DrawCoords(renderer);
    // DrawCurrentInsertMode(renderer);
}

void node::NodeGraphicsScene::SetToolsManager(std::weak_ptr<node::ToolsManager> manager)
{
    m_tools_manager = std::move(manager);
}

void node::NodeGraphicsScene::SetObjectsManager(std::weak_ptr<GraphicsObjectsManager> manager)
{
    m_objects_manager = manager;
}

void node::NodeGraphicsScene::DrawDots(SDL_Renderer* renderer) const
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    const int start_x = static_cast<int>(GetSpaceRect().x / m_dotspace) * m_dotspace;
    const int start_y = static_cast<int>(GetSpaceRect().y / m_dotspace) * m_dotspace;
    auto&& transformer = GetSpaceScreenTransformer();
    const SDL_FPoint start_point_screen = transformer.SpaceToScreenPoint({ start_x, start_y });
    SDL_FPoint step_screen_f = transformer.SpaceToScreenVector({ m_dotspace, m_dotspace });
    SDL_Point step_screen{ static_cast<int>(step_screen_f.x), static_cast<int>(step_screen_f.y) };
    float dot_width = 2;
    static std::vector<SDL_FRect> rects;
    rects.clear();
    if (GetZoomScale() > 1.3)
    {
        dot_width = 1;
    }
    else if (GetZoomScale() < 0.7)
    {
        dot_width = 3;
    }
    for (int i = static_cast<int>(start_point_screen.x); i < GetRect().x + GetRect().w; i += step_screen.x)
    {
        for (int j = static_cast<int>(start_point_screen.y + 5); j < GetRect().y + GetRect().h; j += step_screen.y)
        {
            SDL_FRect rect{ static_cast<float>(i),static_cast<float>(j),dot_width,dot_width };
            rects.push_back(rect);
        }
    }
    SDL_RenderFillRects(renderer, rects.data(), static_cast<int>(rects.size()));

}
void node::NodeGraphicsScene::DrawCurrentInsertMode(SDL_Renderer* renderer) const
{
    UNUSED_PARAM(renderer);
}

bool node::NodeGraphicsScene::OnKeyPress(KeyboardEvent& e)
{
    auto tools_manager = m_tools_manager.lock();
    if (!tools_manager) // we ignore escape if there is no tools manager :(
    {
        return false;
    }
    switch (e.e.type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        switch (e.e.scancode)
        {
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_LCTRL:
        {
            tools_manager->SetTemporaryTool("N");
            return true;
        }
        case SDL_SCANCODE_RSHIFT:
        case SDL_SCANCODE_LSHIFT:
        {
            tools_manager->SetTemporaryTool("D");
            return true;
        }
        case SDL_SCANCODE_ESCAPE:
        {
            CancelCurrentLogic();
            return true;
        }
        case SDL_SCANCODE_Z:
        {
            if (e.e.mod & SDL_KMOD_LCTRL)
            {
                auto mgr = m_objects_manager.lock();
                if (!mgr)
                {
                    return false;
                }
                auto&& model = mgr->GetSceneModel();
                if (!model)
                {
                    return false;
                }
                if (e.e.mod & SDL_KMOD_LSHIFT)
                {
                    if (model->CanRedo())
                    {
                        model->Redo();
                    }
                }
                else
                {
                    if (model->CanUndo())
                    {
                        model->Undo();
                    }
                }
                return true;
            }
            break;
        }
        }
        break;
    }
    case SDL_EVENT_KEY_UP:
    {
        switch (e.e.scancode)
        {
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_LCTRL:
        {
            tools_manager->RemoveTemporaryTool("N");
            return true;
        }
        case SDL_SCANCODE_RSHIFT:
        case SDL_SCANCODE_LSHIFT:
        {
            tools_manager->RemoveTemporaryTool("D");
            return true;
        }
        }
        break;
    }
    }
    return false;
}

void node::NodeGraphicsScene::OnKeyboardFocusIn()
{
    GetApp()->StopTextInput();
}

void node::NodeGraphicsScene::DrawCoords(SDL_Renderer* renderer) const
{
    TTFFont font = TTFFont{ TTF_OpenFont("./assets/FreeSans.ttf", 24) };
    SDL_Color Black = { 122, 122, 122, 255 };
    std::string point_string = std::to_string(m_current_hover_point.x) + std::string(",") + std::to_string(m_current_hover_point.y) +
        std::string(",,") + std::to_string(m_screen_hover_point.x) + std::string(",") + std::to_string(m_screen_hover_point.y);
    SDLSurface message_surface = SDLSurface{ TTF_RenderText_Solid(font.get(), point_string.c_str(), point_string.size(), Black)};
    SDLTexture Message = SDLTexture{ SDL_CreateTextureFromSurface(renderer, message_surface.get()) };

    SDL_FRect Message_rect; //create a rect
    Message_rect.x = GetRect().x + GetRect().w * 2 / 5;  //controls the rect's x coordinate 
    Message_rect.y = GetRect().y + GetRect().h * 4 / 5;; // controls the rect's y coordinte
    Message_rect.w = 400; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    SDL_RenderTexture(renderer, Message.get(), NULL, &Message_rect);
}
MI::ClickEvent node::NodeGraphicsScene::OnRMBUp(MouseButtonEvent& e)
{
    MI::ClickEvent ret =  GraphicsScene::OnRMBUp(e);
    if (ret != MI::ClickEvent::NONE)
    {
        return ret;
    }
    std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(GetApp()->GetScene(), GetApp()->getFont().get());
    GetApp()->GetScene()->ShowContextMenu(std::move(menu), e.point());
    return MI::ClickEvent::CLICKED;
}

void node::NodeGraphicsScene::OnMouseMove(MouseHoverEvent& e)
{
    node::GraphicsScene::OnMouseMove(e);
    SDL_FPoint p{ e.point() };
    auto&& transformer = GetSpaceScreenTransformer();
    m_current_hover_point = transformer.ScreenToSpacePoint(p);
    m_screen_hover_point = p;
}
