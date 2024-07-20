#pragma once

#include "toolgui/MouseInteractable.hpp"

namespace node
{

class BlockObject;
class BlockSocketObject;
class NetSegment;
class NetNode;

class IGraphicsSceneController
{
public:
	virtual MI::ClickEvent OnBlockLMBDown(const SDL_Point& current_mouse_point, BlockObject& node) = 0;
	virtual MI::ClickEvent OnSocketLMBDown(const SDL_Point& current_mouse_point, BlockSocketObject& node) = 0;
	virtual MI::ClickEvent OnSegmentLMBDown(const SDL_Point& current_mouse_point, NetSegment& node) = 0;	
	virtual MI::ClickEvent OnNetNodeLMBDown(const SDL_Point& current_mouse_point, NetNode& node) = 0;


};
}