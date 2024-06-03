#include "GraphicsSceneController.hpp"
#include "GraphicsLogic/NodeDeleteLogic.hpp"
#include "GraphicsScene.hpp"

node::GraphicsSceneController::GraphicsSceneController(GraphicsScene* scene)
    :m_scene{scene}
{
}

MI::ClickEvent node::GraphicsSceneController::OnNodeLMBDown(const SDL_Point& current_mouse_point, Node& node)
{
    UNUSED_PARAM(current_mouse_point);
    if (m_scene->GetMode() == GraphicsSceneMode::Delete)
    {
        m_scene->SetGraphicsLogic(
            std::make_unique<node::NodeDeleteLogic>(node, m_scene)
        );
    }
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsSceneController::OnSocketLMBDown(const SDL_Point& current_mouse_point, NodeSocket& node)
{
    UNUSED_PARAM(current_mouse_point);
    UNUSED_PARAM(node);
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsSceneController::OnSegmentLMBDown(const SDL_Point& current_mouse_point, NetSegment& node)
{
    UNUSED_PARAM(current_mouse_point);
    UNUSED_PARAM(node);
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsSceneController::OnNetNodeLMBDown(const SDL_Point& current_mouse_point, NetNode& node)
{
    UNUSED_PARAM(current_mouse_point);
    UNUSED_PARAM(node);
    return MI::ClickEvent::NONE;
}
