#include "BlockPropertiesDialog.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "BlockClasses/BlockClassesManager.hpp"
#include <charconv>
#include "NodeModels/SceneModelManager.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"


node::BlockPropertiesDialog::BlockPropertiesDialog(const model::BlockModel& block, std::shared_ptr<GraphicsObjectsManager> SceneModel, 
	std::shared_ptr<BlockClassesManager> manager, const SDL_Rect& rect, Scene* parent)
	:Dialog{"Block Properties", rect, parent}, m_scene_manager{std::move(SceneModel)}, m_classesManager{std::move(manager)}, m_block_id{block.GetId()}
{
	assert(parent);
	assert(m_scene_manager);
	assert(m_classesManager);

	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{block.GetClass() + " Block"}, SDL_Rect{ 0,0,100,30 }, parent->GetApp()->getFont().get(), parent));
	auto class_ptr = m_classesManager->GetBlockClassByName(block.GetClass());
	if (class_ptr)
	{
		auto font = parent->GetApp()->getFont(FontType::Label).get();
		auto lines = DialogLabel::SplitToLinesofWidth(std::string{ class_ptr->GetDescription() }, font, 500);
		const int line_height = TTF_FontHeight(font);
		int lines_gap = (lines.size() == 0) ? 0 : DialogLabel::LinesMargin * static_cast<int>(lines.size() - 1);
		AddControl(std::make_unique<DialogLabel>(std::move(lines), SDL_Rect{ 0,0,500, line_height * static_cast<int>(lines.size()) + lines_gap }, font, parent));
	}

	if (block.GetProperties().size())
	{
		AddControl(std::make_unique<SeparatorControl>(SDL_Rect{ 0,0,500, 2 }, parent));
	}

	for (const auto& property : block.GetProperties())
	{
		std::string initial_value = property.to_string();
		auto ptr = std::make_unique<PropertyEditControl>(property.name, 200, std::move(initial_value), SDL_Rect{ 0,0,500, 35 }, parent);
		m_property_edits.push_back(BlockPropertySlot{ ptr.get(),
			[old_prop = property](const std::string& value) ->std::optional<model::BlockProperty>
			{ 
				switch (old_prop.type)
				{
					using enum model::BlockPropertyType;
				case Integer:
				{
					int64_t new_value = 0;
					auto res = std::from_chars(value.data(), value.data() + value.size(), new_value);
					if (res.ec != std::errc{})
					{
						return std::nullopt;
					}
					auto new_prop = old_prop;
					new_prop.prop = new_value;
					return new_prop;
				}
				case UnsignedInteger:
				{
					uint64_t new_value = 0;
					auto res = std::from_chars(value.data(), value.data() + value.size(), new_value);
					if (res.ec != std::errc{})
					{
						return std::nullopt;
					}
					auto new_prop = old_prop;
					new_prop.prop = new_value;
					return new_prop;
				}	
				case FloatNumber:
				{
					double new_value = 0;
					auto res = std::from_chars(value.data(), value.data() + value.size(), new_value);
					if (res.ec != std::errc{})
					{
						return std::nullopt;
					}
					auto new_prop = old_prop;
					new_prop.prop = new_value;
					return new_prop;
				}
				case Boolean:
				{
					int64_t new_value = 0;
					auto res = std::from_chars(value.data(), value.data() + value.size(), new_value);
					if (res.ec != std::errc{})
					{
						return std::nullopt;
					}
					auto new_prop = old_prop;
					new_prop.prop = new_value != 0;
					return new_prop;
				}
				case String:
				{
					auto new_prop = old_prop;
					new_prop.prop = value;
					return new_prop;
				}
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
			SDL_Log("Update Failed!");
			Dialog::OnOk();
			return;
		}
	}

	bool renew_sockets = false;
	std::vector<model::BlockSocketModel> new_sockets;
	auto block = m_scene_manager->GetSceneModel()->GetModel().GetBlockById(m_block_id);
	if (block)
	{
		auto block_class = m_classesManager->GetBlockClassByName(block->get().GetClass());
		if (block_class)
		{
			auto new_sockets_type = block_class->CalculateSockets(new_properties);
			auto old_sockets = block->get().GetSockets();
			if (new_sockets_type.size() != old_sockets.size())
			{
				renew_sockets = true;
			}
			else
			{
				for (size_t i = 0; i < new_sockets_type.size(); i++)
				{
					if (new_sockets_type[i] != old_sockets[i].GetType())
					{
						renew_sockets = true;
						break;
					}
				}
			}
			if (renew_sockets)
			{
				new_sockets.reserve(new_sockets_type.size());
				for (size_t i = 0; i < new_sockets_type.size(); i++)
				{
					new_sockets.push_back(model::BlockSocketModel{ new_sockets_type[i],model::SocketId{static_cast<model::id_int>(i)} });
				}
				auto&& block_registry = m_scene_manager->getBlocksRegistry();
				auto it = block_registry.find(m_block_id);
				assert(it != block_registry.end());
				if (it != block_registry.end())
				{
					it->second->GetStyler().PositionSockets(new_sockets, block->get().GetBounds());
				}
			}
		}
	}
	auto model_manager = m_scene_manager->GetSceneModel();
	auto block_id = m_block_id;
	Dialog::OnOk();
	if (renew_sockets)
	{
		model_manager->ModifyBlockPropertiesAndSockets(block_id, std::move(new_properties), std::move(new_sockets));
	}
	else
	{
		model_manager->ModifyBlockProperties(block_id, std::move(new_properties));
	}
	SDL_Log("Update Done!");
	return;
}

std::vector<std::string> node::DialogLabel::SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width)
{
	std::vector<std::string> lines;
	std::string_view sv{ str };
	while (sv.size())
	{
		int captured_letters = 0;
		int extent = 0;
		int result = TTF_MeasureUTF8(font, sv.data(), width, &extent, &captured_letters);
		if (result)
		{
			SDL_Log("Failed to measure font extent");
			return lines;
		}
		if (static_cast<size_t>(captured_letters) == sv.size())
		{
			lines.push_back(std::string{ sv });
			return lines;
		}
		else
		{
			auto pos = sv.rfind(' ', captured_letters);
			if (pos == sv.npos)
			{
				lines.push_back(std::string{ sv.substr(0,captured_letters) });
				sv = sv.substr(captured_letters);
			}
			else
			{
				lines.push_back(std::string{ sv.substr(0,pos) });
				sv = sv.substr(pos + 1);
			}

		}
	}
	return lines;
}

node::DialogLabel::DialogLabel(std::vector<std::string> lines, const SDL_Rect& rect, TTF_Font* font, Scene* parent)
	:DialogControl{rect, parent}, m_lines{std::move(lines)}, m_font{font}
{
	assert(m_font);
	assert(parent);
}

void node::DialogLabel::Draw(SDL_Renderer* renderer)
{
	int y = 0;

	if (!m_painters.size())
	{
		m_painters.reserve(m_lines.size());
		for (const auto& line : m_lines)
		{
			UNUSED_PARAM(line);
			m_painters.emplace_back(m_font);
		}
	}
	int font_height = TTF_FontHeight(m_font);
	assert(m_lines.size() == m_painters.size());
	for (size_t i = 0; i < m_lines.size(); i++)
	{
		const auto& line = m_lines[i];
		auto&& painter = m_painters[i];

		SDL_Color Black = { 50, 50, 50, 255 };
		painter.SetText(line);

		SDL_Point text_start{ GetRect().x, GetRect().y + y };
		painter.Draw(renderer, text_start, Black);
		y += font_height + LinesMargin;
	}

}

node::PropertyEditControl::PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent)
	:DialogControl{ rect, parent }, 
	m_edit{ std::move(initial_value), {rect.x + m_name_width, rect.y, rect.w - m_name_width, rect.h}, parent }, 
	m_name{ std::move(name) },
	m_painter{ GetScene()->GetApp()->getFont().get() },
	m_name_width{ name_width }
{
	m_painter.SetText(m_name);
	assert(parent);
}

void node::PropertyEditControl::Draw(SDL_Renderer* renderer)
{
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		SDL_Point text_start{ GetRect().x, GetRect().y };
		m_painter.Draw(renderer, text_start, Black);
	}
	m_edit.Draw(renderer);
}

void node::PropertyEditControl::OnSetRect(const SDL_Rect& rect)
{
	DialogControl::OnSetRect(rect);
	m_edit.SetRect({ rect.x + m_name_width, rect.y, rect.w - m_name_width, rect.h });
}

node::Widget* node::PropertyEditControl::OnGetInteractableAtPoint(const SDL_Point& point)
{
	if (auto ptr = m_edit.GetInteractableAtPoint(point))
	{
		return ptr;
	}
	return this;
}

void node::SeparatorControl::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
}
