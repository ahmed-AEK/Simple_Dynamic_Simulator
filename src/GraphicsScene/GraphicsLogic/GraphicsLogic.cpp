#include "GraphicsLogic.hpp"

node::GraphicsLogic::GraphicsLogic(GraphicsScene* scene)
	:m_pScene(scene)
{
}

node::GraphicsLogic::~GraphicsLogic()
{
}

void node::GraphicsLogic::setScene(GraphicsScene* scene)
{
	m_pScene = scene;
}


void node::GraphicsLogic::MouseOut()
{
    this->OnMouseOut();
}

void node::GraphicsLogic::MouseIn()
{
    this->OnMouseIn();
}

void node::GraphicsLogic::MouseMove(const SDL_Point& current_mouse_point)
{
    this->OnMouseMove(current_mouse_point);
}

MI::ClickEvent node::GraphicsLogic::LMBDown(const SDL_Point& current_mouse_point)
{
    return this->OnLMBDown(current_mouse_point);
}

MI::ClickEvent node::GraphicsLogic::RMBDown(const SDL_Point& current_mouse_point)
{
    return this->OnRMBDown(current_mouse_point);
}

MI::ClickEvent node::GraphicsLogic::RMBUp(const SDL_Point& current_mouse_point)
{
    return this->OnRMBUp(current_mouse_point);
}

void node::GraphicsLogic::Cancel()
{
    if (!b_done)
    {
        OnCancel();
    }
}

MI::ClickEvent node::GraphicsLogic::LMBUp(const SDL_Point& current_mouse_point)
{
    return this->OnLMBUp(current_mouse_point);
}

void node::GraphicsLogic::OnMouseOut()
{
}

void node::GraphicsLogic::OnMouseIn()
{
}

void node::GraphicsLogic::OnMouseMove(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}

MI::ClickEvent node::GraphicsLogic::OnLMBDown(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsLogic::OnRMBDown(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsLogic::OnLMBUp(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsLogic::OnRMBUp(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return MI::ClickEvent::NONE;
}

void node::GraphicsLogic::OnCancel()
{
}

