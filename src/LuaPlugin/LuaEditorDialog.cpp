#include "LuaEditorDialog.hpp"

#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/ToolBar.hpp"

#include "boost/container/static_vector.hpp"

std::unique_ptr<node::LuaEditorDialog> node::LuaEditorDialog::Create(const model::BlockModel& block, 
	std::shared_ptr<IBlockPropertiesUpdater> model_updater,
	BlockClassPtr block_class,
	std::string property_name, const WidgetSize& size, Scene* parent)
{
	auto block_data = model_updater->GetFunctionalBlockData();
	if (!block_data)
	{
		return nullptr;
	}
	auto it = std::find_if(block_data->properties.begin(), block_data->properties.end(), [&](const model::BlockProperty& prop) {return prop.name == property_name; });
	if (it == block_data->properties.end())
	{
		return nullptr;
	}
	if (!it->get_str())
	{
		return nullptr;
	}

	return std::make_unique<LuaEditorDialog>(block.GetId(), std::move(model_updater), std::move(block_class), std::move(property_name), *it->get_str(), size, parent);
}

node::LuaEditorDialog::LuaEditorDialog(const model::BlockId& block_id,
	std::shared_ptr<IBlockPropertiesUpdater> model_updater,
	BlockClassPtr block_class,
	std::string property_name, std::string code, const WidgetSize& size, Scene* parent)
	:BlockDialog{"Lua Editor", size, parent}, m_model_updater{std::move(model_updater)},
	m_block_class{std::move(block_class)}, m_property_name{std::move(property_name)},
	m_block_id{block_id}
{	
	SetResizeable(true);
	auto editor = std::make_unique<LuaCodeEditor>(parent->GetApp()->getFont(FontType::Label).get(),
		WidgetSize{200.0f,200.0f}, this);
	m_editor = editor.get();
	m_editor->SetText(std::move(code));
	AddControl(std::move(editor));
	AddButton("Apply", [this]() {this->UpdateProperties(); });
	AddButton("Ok", [this]() {this->TriggerOk(); });
	AddButton("Cancel", [this]() {this->TriggerClose(); });
}

void node::LuaEditorDialog::UpdateResults(std::any new_result)
{
	UNUSED_PARAM(new_result);
}

void node::LuaEditorDialog::OnOk()
{
	if (!this->UpdateProperties())
	{
		return;
	}
	BlockDialog::OnOk();
}

bool node::LuaEditorDialog::UpdateProperties()
{
	auto block_data = m_model_updater->GetFunctionalBlockData();
	if (!block_data)
	{
		m_logger.LogError("Update Failed! no block data found!");
		return false;
	}
	std::vector<model::BlockProperty> new_properties = std::move(block_data->properties);

	for (auto& property : new_properties)
	{
		if (property.name == m_property_name)
		{
			property.prop = m_editor->GetText();
		}
	}
	auto result = m_model_updater->UpdateBlockProperties(new_properties);
	if (result)
	{
		m_logger.LogInfo("Update Done!");
		return true;
	}
	else
	{
		m_logger.LogError("Update Failed!");
		for (auto& reason : result.error())
		{
			m_logger.LogError("error at index: {} : {}", reason.prop_idx, reason.error_text);
		}
		return false;
	}
}

namespace detail
{
	template <typename T, typename U, std::size_t ... Is>
	constexpr std::array<T, sizeof...(Is)>
		create_array(U value, std::index_sequence<Is...>)
	{
		// cast Is to void to remove the warning: unused value
		return { {(static_cast<void>(Is), T{value})...} };
	}
}

template <std::size_t N, typename T, typename U>
constexpr std::array<T, N> create_array(const U& value)
{
	return detail::create_array<T>(value, std::make_index_sequence<N>());
}

node::LuaCodeEditor::LuaCodeEditor(TTF_Font* font, const WidgetSize& size, Dialog* parent)
	:DialogControl{ size,parent }, m_font{ font }, m_edit{size, font, this}
{
	SetSizingMode(DialogControl::SizingMode::expanding);
}

void node::LuaCodeEditor::SetText(std::string text)
{
	std::vector<std::string> lines;
	std::string_view current_str{ text };
	auto pos = current_str.find('\n');
	while (pos != current_str.npos)
	{
		lines.push_back(std::string(current_str.substr(0, pos)));
		current_str = current_str.substr(pos);
		if (current_str.size())
		{
			current_str = current_str.substr(1);
		}
		pos = current_str.find('\n');
	}
	lines.push_back(std::string(current_str));
	m_edit.SetText(std::move(lines));
}

std::string node::LuaCodeEditor::GetText() const
{
	std::string ret;
	size_t total_size = 0;
	for (auto& str : m_edit.GetText())
	{
		total_size += str.size() + 1;
	}
	ret.reserve(total_size);
	for (auto& str : m_edit.GetText())
	{
		ret.insert(ret.end(), str.begin(), str.end());
		ret.insert(ret.end(), '\n');
	}
	return ret;
}

void node::LuaCodeEditor::OnSetSize(const WidgetSize& size)
{
	DialogControl::OnSetSize(size);
	m_edit.SetSize(GetSize());
}

