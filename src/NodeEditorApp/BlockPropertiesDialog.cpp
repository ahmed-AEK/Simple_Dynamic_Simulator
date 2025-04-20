#include "BlockPropertiesDialog.hpp"
#include "PluginAPI/BlockClassesManager.hpp"


#include "NodeSDLStylers/BlockStyler.hpp"
#include "toolgui/DialogControls.hpp"

static std::string GetClassDescription(node::IBlockClass& block)
{
	std::string ret;
	block.GetDescription([](void* context, std::string_view description) { *static_cast<std::string*>(context) = std::string{ description }; }, &ret);
	return ret;
}

node::BlockPropertiesDialog::BlockPropertiesDialog(std::shared_ptr<IBlockPropertiesUpdater> updater, 
	BlockClassPtr block_class,
	const model::FunctionalBlockData& block_data,
	const WidgetSize& size, Scene* parent)
	:Dialog{ "Block Properties", size, parent }, m_updater{ std::move(updater)}, 
	m_block_class{ std::move(block_class) }
{
	assert(parent);
	assert(m_updater);
	assert(m_block_class);

	TTF_Font* font_title = GetApp()->getFont().get();
	int font_height = TTF_GetFontHeight(font_title);

	{
		std::string title = block_data.block_class + " Block";
		int measured_width = 0;
		size_t measured_length = 0;
		TTF_MeasureString(font_title, title.c_str(), title.size(), 500, &measured_width, &measured_length);

		AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{std::move(title)},
			WidgetSize{ static_cast<float>(measured_width),static_cast<float>(font_height) }, font_title, this));
	}

	auto* font_label = parent->GetApp()->getFont(FontType::Label).get();
	assert(m_block_class);
	if (m_block_class)
	{
		auto lines = DialogLabel::SplitToLinesofWidth(GetClassDescription(*m_block_class), font_label, 500);
		const int line_height = TTF_GetFontHeight(font_label);
		int lines_gap = (lines.size() == 0) ? 0 : DialogLabel::LinesMargin * static_cast<int>(lines.size() - 1);
		AddControl(std::make_unique<DialogLabel>(std::move(lines), 
			WidgetSize{ 500.0f, static_cast<float>(line_height * static_cast<int>(lines.size()) + lines_gap) }, font_label, this));
	}

	if (block_data.properties.size())
	{
		AddControl(std::make_unique<SeparatorControl>(WidgetSize{ 500.0f, 2.0f }, this));
	}
	for (const auto& property : block_data.properties)
	{
		std::string initial_value = property.to_string();
		auto ptr = std::make_unique<PropertyEditControl>(property.name, 200, std::move(initial_value), 
			WidgetSize{ 500.0f, 60.0f }, font_title, font_label, this);
		m_property_edits.push_back(BlockPropertySlot{ ptr.get(),
			[old_prop = property](const std::string& value) ->std::optional<model::BlockProperty>
			{
				if (auto prop = model::BlockProperty::from_string(old_prop.GetType(), value))
				{
					auto new_prop = old_prop;
					new_prop.prop = std::move(*prop);
					return new_prop;
				}
				return std::nullopt;
			}
			});
		AddControl(std::move(ptr));
	}
	AddButton("Ok", [this]() {this->TriggerOk(); });
	AddButton("Cancel", [this]() {this->TriggerClose(); });
}

void node::BlockPropertiesDialog::OnOk()
{
	std::vector<model::BlockProperty> new_properties;
	for (auto&& control : m_property_edits)
	{
		if (auto value = control.grabber(control.property_edit->GetValue()))
		{
			new_properties.push_back(*value);
		}
		else
		{
			m_logger.LogError("Update Failed!");
			m_logger.LogError("Bad Value For Property: {}", control.property_edit->GetName());
			for (const auto& prop : m_property_edits)
			{
				prop.property_edit->SetErrorText({});
			}
			control.property_edit->SetErrorText({ "Wrong property type" });
			return;
		}
	}

	auto result = m_updater->UpdateBlockProperties(new_properties);
	if (result)
	{
		Dialog::OnOk();
	}
	else
	{
		for (const auto& prop : m_property_edits)
		{
			prop.property_edit->SetErrorText({});
		}
		for (auto& error_text : result.error())
		{
			if (m_property_edits.size() <= error_text.prop_idx)
			{
				m_logger.LogError("invalid Error index: {}", static_cast<int>(error_text.prop_idx));
				continue;
			}
			m_property_edits[error_text.prop_idx].property_edit->SetErrorText({ std::move(error_text.error_text) });
		}
		m_logger.LogError("Update Failed class verification!");
		return;
	}
	return;
}
