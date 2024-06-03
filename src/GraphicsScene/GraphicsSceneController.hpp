#pragma once

#include "GraphicsScene/IGraphicsSceneController.hpp"

namespace node
{

class GraphicsScene;

class GraphicsSceneController : public IGraphicsSceneController
{
public:
	explicit GraphicsSceneController(GraphicsScene* scene);
	MI::ClickEvent OnNodeLMBDown(const SDL_Point& current_mouse_point, Node& node) override;
	MI::ClickEvent OnSocketLMBDown(const SDL_Point& current_mouse_point, NodeSocket& node) override;
	MI::ClickEvent OnSegmentLMBDown(const SDL_Point& current_mouse_point, NetSegment& node) override;
	MI::ClickEvent OnNetNodeLMBDown(const SDL_Point& current_mouse_point, NetNode& node) override;

protected:
	GraphicsScene* m_scene;
};
}