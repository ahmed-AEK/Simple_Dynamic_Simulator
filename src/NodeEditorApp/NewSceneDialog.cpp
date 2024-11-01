#include "NewSceneDialog.hpp"
#include "toolgui/DialogControls.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "MainNodeScene.hpp"
#include "boost/algorithm/string.hpp"


node::OkCancelModalDialog::OkCancelModalDialog(std::string title, std::vector<std::string> content, const SDL_FRect& rect, MainNodeScene* parent, bool hide_cancel)
	:Dialog{ std::move(title), rect, parent }
{
	assert(parent);
	auto* font = parent->GetApp()->getFont().get();
	int max_width = 100;
	int total_height = 0;
	int width = 100;
	int height = 30;
	for (auto&& line : content)
	{
		if (!TTF_GetStringSize(font, line.c_str(), line.size(), &width, &height))
		{
			SDL_Log("Failed to measure font extent");
		}
		max_width = std::max(width, max_width);
		total_height += height;
	}
	total_height += content.size() ? static_cast<int>(DialogLabel::LinesMargin * content.size() - 1) : 0;

	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{std::move(content)}, SDL_FRect{ 0.0f,0.0f, static_cast<float>(width),static_cast<float>(total_height) }, font, this));
	AddButton("Ok", [this] {this->TriggerOk(); });
	if (!hide_cancel)
	{
		AddButton("Cancel", [this] {this->TriggerClose(); });
	}
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


node::SingleEntryDialog::SingleEntryDialog(std::string title, std::vector<std::string> content, std::string initial_value, const SDL_FRect& rect, MainNodeScene* parent)
	:OkCancelModalDialog{ std::move(title), std::move(content), rect, parent }
{
	assert(parent);
	auto edit = std::make_unique<PropertyEditControl>("", 0, std::move(initial_value), SDL_FRect{0.0f,0.0f,500.0f, 35.0f}, this);
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

node::LoadSceneDialog::LoadSceneDialog(const SDL_FRect& rect, MainNodeScene* parent)
	:SingleEntryDialog{ "Load Scene", {"Loaded Scene name:"}, "saved_scene.blks", rect, parent }
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
	scene->LoadScene(std::move(data));
}

node::NewSceneDialog::NewSceneDialog(const SDL_FRect& rect, MainNodeScene* parent)
	:OkCancelModalDialog{ "Create New Scene", {"Create a new scene ?","Unsaved work will be lost!"}, rect, parent }
{
}

void node::NewSceneDialog::OnOk()
{
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	assert(scene);
	OkCancelModalDialog::OnOk();
	scene->NewScene();
}

node::SaveSceneDialog::SaveSceneDialog(const SDL_FRect& rect, MainNodeScene* parent)
	:SingleEntryDialog{ "Save Scene", {"Saved Scene name:"}, "saved_scene.blks", rect, parent }
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
	scene->MaybeSaveScene(std::move(data));
}

node::ConfirmOverwriteSaveSceneDialog::ConfirmOverwriteSaveSceneDialog(std::string name, const SDL_FRect& rect, MainNodeScene* parent)
	: OkCancelModalDialog{ "Overwrite File!", {"File already exists!", "Overwrite file : " + name}, rect, parent }, m_name{ std::move(name) }
{
}

void node::ConfirmOverwriteSaveSceneDialog::OnOk()
{
	std::string name = std::move(m_name);
	auto* scene = static_cast<MainNodeScene*>(GetScene());
	OkCancelModalDialog::OnOk();
	scene->SaveScene(std::move(name));
}
