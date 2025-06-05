#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class NetTool : public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnObjectLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object) override;
	bool IsObjectClickable(GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object) override;

private:
};


}