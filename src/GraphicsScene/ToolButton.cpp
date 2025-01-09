#include "ToolButton.hpp"
#include "ToolsManager.hpp"

node::ToolButton::ToolButton(const WidgetSize& size, ToolBar* parent, 
	std::string name, std::weak_ptr<ToolsManager> manager)
	:ToolBarButton{size, parent, std::move(name)}, m_manager{std::move(manager)}
{
}

void node::ToolButton::OnButonClicked()
{
	if (auto ptr = m_manager.lock())
	{
		ptr->ChangeTool(GetName());
	}
}
