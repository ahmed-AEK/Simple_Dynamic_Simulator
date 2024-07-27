#pragma once

#include "toolgui/ToolBar.hpp"

namespace node
{

class ToolsManager;

class ToolButton : public ToolBarButton
{
public:
	ToolButton(const SDL_Rect& rect, Scene* parent, 
		std::string name = {}, std::weak_ptr<ToolsManager> manager = {});
	void OnButonClicked() override;
private:
	std::weak_ptr<ToolsManager> m_manager;
};

}