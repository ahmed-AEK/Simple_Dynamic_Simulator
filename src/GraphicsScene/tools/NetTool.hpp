#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class NetTool : public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(const model::Point & p) override;
private:
};


}