#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class GraphicsObject;

class DeleteTool: public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
private:
};
}