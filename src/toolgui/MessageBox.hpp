#pragma once

#include "toolgui/Dialog.hpp"
#include "functional"

namespace node
{

class MessageBoxDialog : public Dialog
{
public:
	MessageBoxDialog(std::string title, std::string content, const SDL_Rect& rect = {300,300,300,200}, Scene* parent = nullptr);

	bool SetOnOk(std::function<void()> callback);
private:
	std::function<void()> m_onOk;
	std::string m_content;
};
}