#pragma once

#include "toolgui/MouseInteractable.hpp"

namespace node
{

class Node;
class NodeSocket;
class NetSegment;
class NetNode;

class IGraphicsSceneController
{
public:
	virtual MI::ClickEvent OnNodeLMBDown(const SDL_Point& current_mouse_point, Node& node) = 0;
	virtual MI::ClickEvent OnSocketLMBDown(const SDL_Point& current_mouse_point, NodeSocket& node) = 0;
	virtual MI::ClickEvent OnSegmentLMBDown(const SDL_Point& current_mouse_point, NetSegment& node) = 0;	
	virtual MI::ClickEvent OnNetNodeLMBDown(const SDL_Point& current_mouse_point, NetNode& node) = 0;


};
}