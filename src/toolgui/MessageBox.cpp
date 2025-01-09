#include "MessageBox.hpp"

node::MessageBoxDialog::MessageBoxDialog(std::string title, std::string content, 
	const WidgetSize& size, Scene* parent)
	:Dialog{std::move(title),size, parent}, m_content{std::move(content)}
{
}

bool node::MessageBoxDialog::SetOnOk(std::function<void()> callback)
{
	if (m_onOk)
	{
		return false;
	}
	m_onOk = std::move(callback);
	return true;
}
