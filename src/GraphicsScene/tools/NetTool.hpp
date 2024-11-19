#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class NetTool : public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager) override;
private:
};


}