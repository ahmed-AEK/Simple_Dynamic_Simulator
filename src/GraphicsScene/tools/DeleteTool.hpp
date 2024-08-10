#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class GraphicsObject;

class DeleteTool: public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(const model::Point& p) override;
	MI::ClickEvent OnLMBUp(const model::Point& p) override;
private:
};
}