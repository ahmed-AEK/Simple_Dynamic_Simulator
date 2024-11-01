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
#include "toolgui/DialogControls.hpp"

node::BlockPropertiesDialog::BlockPropertiesDialog(const model::BlockModel& block, std::shared_ptr<GraphicsObjectsManager> SceneModel, 
	std::shared_ptr<BlockClassesManager> manager, const SDL_FRect& rect, Scene* parent)
	:Dialog{"Block Properties", rect, parent}, m_scene_manager{std::move(SceneModel)}, m_classesManager{std::move(manager)}, m_block_id{block.GetId()}
{
	assert(parent);
	assert(m_scene_manager);
	assert(m_classesManager);
	TTF_Font* font_title = GetApp()->getFont().get();
	int font_height = TTF_GetFontHeight(font_title);
	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{block.GetClass() + " Block"}, SDL_FRect{ 0.0f,0.0f,100.0f,static_cast<float>(font_height) }, font_title, this));
	auto class_ptr = m_classesManager->GetBlockClassByName(block.GetClass());
	assert(class_ptr);
	if (class_ptr)
	{
		auto font = parent->GetApp()->getFont(FontType::Label).get();
		auto lines = DialogLabel::SplitToLinesofWidth(std::string{ class_ptr->GetDescription() }, font, 500);
		const int line_height = TTF_GetFontHeight(font);
		int lines_gap = (lines.size() == 0) ? 0 : DialogLabel::LinesMargin * static_cast<int>(lines.size() - 1);
		AddControl(std::make_unique<DialogLabel>(std::move(lines), SDL_FRect{ 0.0f,0.0f,500.0f, static_cast<float>(line_height * static_cast<int>(lines.size()) + lines_gap) }, font, this));
	}

	if (block.GetProperties().size())
	{
		AddControl(std::make_unique<SeparatorControl>(SDL_FRect{ 0.0f,0.0f,500.0f, 2.0f }, this));
	}

	for (const auto& property : block.GetProperties())
	{
		std::string initial_value = property.to_string();
		auto ptr = std::make_unique<PropertyEditControl>(property.name, 200, std::move(initial_value), SDL_FRect{ 0.0f,0.0f,500.0f, static_cast<float>(font_height + 5) }, this);
		m_property_edits.push_back(BlockPropertySlot{ ptr.get(),
			[old_prop = property](const std::string& value) ->std::optional<model::BlockProperty>
			{
				if (auto prop = model::BlockProperty::from_string(old_prop.type, value))
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
			SDL_Log("Update Failed!");
			SDL_Log("Bad Value For Property: %s", control.property_edit->GetName().c_str());
			return;
		}
	}

	auto block = m_scene_manager->GetSceneModel()->GetModel().GetBlockById(m_block_id);
	if (!block)
	{
		Dialog::OnOk();
		SDL_Log("Update Failed!");
		return;
	}
	auto block_class = m_classesManager->GetBlockClassByName(block->get().GetClass());
	if (!block_class)
	{
		Dialog::OnOk();
		SDL_Log("Update Failed!");
		return;
	}

	bool renew_sockets = false;
	std::vector<model::BlockSocketModel> new_sockets;
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
			it->second->GetStyler().PositionSockets(new_sockets, block->get().GetBounds(), block->get().GetOrienation());
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
