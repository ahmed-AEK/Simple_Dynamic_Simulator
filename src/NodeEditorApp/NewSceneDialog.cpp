#include "NewSceneDialog.hpp"
#include "toolgui/DialogControls.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "MainNodeScene.hpp"
#include "boost/algorithm/string.hpp"


node::OkCancelModalDialog::OkCancelModalDialog(std::string title, std::string content, const SDL_Rect& rect, MainNodeScene* parent)
	:Dialog{ std::move(title), rect, parent }
{
	assert(parent);
	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{std::move(content)}, SDL_Rect{ 0,0,100,30 }, parent->GetApp()->getFont().get(), parent));
	AddButton("Ok", [this] {this->TriggerOk(); });
	AddButton("Cancel", [this] {this->TriggerClose(); });
}

void node::OkCancelModalDialog::OnOk()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	scene->SetModalDialog(nullptr);
}

void node::OkCancelModalDialog::OnClose()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	scene->SetModalDialog(nullptr);
}


node::SingleEntryDialog::SingleEntryDialog(std::string title, std::string content, std::string initial_value, const SDL_Rect& rect, MainNodeScene* parent)
	:OkCancelModalDialog{std::move(title), std::move(content), rect, parent}
{
	assert(parent);
	auto edit = std::make_unique<PropertyEditControl>("", 0, std::move(initial_value), SDL_Rect{0,0,500, 35}, parent);
	m_edit = edit.get();
	AddControl(std::move(edit));
}

void node::SingleEntryDialog::OnOk()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	scene->SetModalDialog(nullptr);
}

void node::SingleEntryDialog::OnClose()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	scene->SetModalDialog(nullptr);
}

std::string node::SingleEntryDialog::GetValue()
{
	return m_edit->GetValue();
}

node::LoadSceneDialog::LoadSceneDialog(const SDL_Rect& rect, MainNodeScene* parent)
	:SingleEntryDialog{ "Load Scene", "Please enter the loaded Scene name:", "loaded_scene.blks", rect, parent }
{
}

void node::LoadSceneDialog::OnOk()
{
	auto data = GetValue();
	boost::trim(data);
	if (!data.size())
	{
		return;
	}
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	SingleEntryDialog::OnOk();
	scene->SetModalDialog(nullptr);
	scene->LoadScene(std::move(data));
}

node::NewSceneDialog::NewSceneDialog(const SDL_Rect& rect, MainNodeScene* parent)
	:OkCancelModalDialog{"Create New Scene", "create a new scene ?", rect, parent}
{
}

void node::NewSceneDialog::OnOk()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	OkCancelModalDialog::OnOk();
	scene->SetModalDialog(nullptr);
	scene->NewScene();
}

node::SaveSceneDialog::SaveSceneDialog(const SDL_Rect& rect, MainNodeScene* parent)
	:SingleEntryDialog{ "Save Scene", "Please enter the saved Scene name:", "saved_scene.blks", rect, parent }
{
}

void node::SaveSceneDialog::OnOk()
{
	auto data = GetValue();
	boost::trim(data);
	if (!data.size())
	{
		return;
	}
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	SingleEntryDialog::OnOk();
	scene->SetModalDialog(nullptr);
	scene->SaveScene(std::move(data));
}
