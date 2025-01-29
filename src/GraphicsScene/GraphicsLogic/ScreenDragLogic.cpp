#include "ScreenDragLogic.hpp"
#include "GraphicsScene.hpp"

node::logic::ScreenDragLogic::ScreenDragLogic(SDL_FPoint startPointScreen, 
    node::model::Point startEdgeSpace, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_startPointScreen{startPointScreen}, m_startEdgeSpace{startEdgeSpace}
{
}

void node::logic::ScreenDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
    GraphicsScene* scene = GetScene();
    assert(scene);
    auto&& transformer = scene->GetSpaceScreenTransformer();
    SDL_FPoint point = transformer.SpaceToScreenPoint(current_mouse_point);
    const auto current_position_difference_space_vector = transformer.ScreenToSpaceVector({
    point.x - m_startPointScreen.x, point.y - m_startPointScreen.y
        });

    auto&& space_rect = scene->GetSpaceRect();
    scene->SetSpaceRect({
        m_startEdgeSpace.x - current_position_difference_space_vector.x,
        m_startEdgeSpace.y - current_position_difference_space_vector.y,
        space_rect.w,
        space_rect.h
        });
}

void node::logic::ScreenDragLogic::OnCancel()
{
    GraphicsScene* scene = GetScene();
    assert(scene);
    auto&& space_rect = scene->GetSpaceRect();
    scene->SetSpaceRect({
        m_startEdgeSpace.x,
        m_startEdgeSpace.y,
        space_rect.w,
        space_rect.h
        });
}


