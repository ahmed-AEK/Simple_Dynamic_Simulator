#include "AboutDialog.hpp"
#include "toolgui/DialogControls.hpp"

#ifndef APP_VERSION_MAJOR
#define APP_VERSION_MAJOR 0
#endif

#ifndef APP_VERSION_MINOR
#define APP_VERSION_MINOR 0
#endif

#ifndef APP_VERSION_PATCH
#define APP_VERSION_PATCH 0
#endif

node::AboutDialog::AboutDialog(const WidgetSize& size, Scene* parent)
	:Dialog{"About Application", size, parent}
{
	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{
		{ "Dynamic Simulator"},
		{ "Version: " + std::to_string(APP_VERSION_MAJOR) + "." + std::to_string(APP_VERSION_MINOR) + 
			"." + std::to_string(APP_VERSION_PATCH) },
		{ "This is a free and open source application." },
		{ "This software is provided without any form of warranty." }},
		WidgetSize{ 500.0f,120.0f }, parent->GetApp()->getFont().get(), this));
	AddButton("Ok", [this]() { this->TriggerOk(); });
}
