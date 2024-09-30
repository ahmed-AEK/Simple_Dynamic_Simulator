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
	void SetActive(bool value = true) { b_active = value; }
protected:
	bool IsDisabled() override { return b_active; }
private:
	std::weak_ptr<ToolsManager> m_manager;
	bool b_active = false;
};

}