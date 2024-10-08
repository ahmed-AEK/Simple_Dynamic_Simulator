#include "GraphicsLogic.hpp"

node::logic::GraphicsLogic::GraphicsLogic(GraphicsScene* scene, GraphicsObjectsManager* manager)
    :m_pScene(scene), m_objects_manager{manager}
{
}

node::logic::GraphicsLogic::~GraphicsLogic()
{
}

void node::logic::GraphicsLogic::setScene(GraphicsScene* scene)
{
	m_pScene = scene;
}


void node::logic::GraphicsLogic::MouseOut()
{
    this->OnMouseOut();
}

void node::logic::GraphicsLogic::MouseIn()
{
    this->OnMouseIn();
}

void node::logic::GraphicsLogic::Start(const model::Point& current_mouse_point)
{
    OnStart(current_mouse_point);
}

void node::logic::GraphicsLogic::MouseMove(const model::Point& current_mouse_point)
{
    this->OnMouseMove(current_mouse_point);
}

void node::logic::GraphicsLogic::Cancel()
{
    if (!b_done)
    {
        OnCancel();
    }
}

MI::ClickEvent node::logic::GraphicsLogic::LMBUp(const model::Point& current_mouse_point)
{
    return this->OnLMBUp(current_mouse_point);
}

void node::logic::GraphicsLogic::OnStart(const model::Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}

void node::logic::GraphicsLogic::OnMouseOut()
{
}

void node::logic::GraphicsLogic::OnMouseIn()
{
}

void node::logic::GraphicsLogic::OnMouseMove(const model::Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}


MI::ClickEvent node::logic::GraphicsLogic::OnLMBUp(const model::Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return MI::ClickEvent::CAPTURE_END;
}


void node::logic::GraphicsLogic::OnCancel()
{
}

