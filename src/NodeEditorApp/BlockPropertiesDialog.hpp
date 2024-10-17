#pragma once

#include "toolgui/Dialog.hpp"

class SDLFont;

namespace node
{
class BlockClassesManager;
class GraphicsObjectsManager;
class PropertyEditControl;

class BlockPropertiesDialog: public Dialog
{
public:
	BlockPropertiesDialog(const model::BlockModel& block, std::shared_ptr<GraphicsObjectsManager> SceneModel, std::shared_ptr<BlockClassesManager> manager, const SDL_Rect& rect, Scene* parent);
protected:
	void OnOk() override;
private:
	struct BlockPropertySlot
	{
		PropertyEditControl* property_edit;
		std::function<std::optional<model::BlockProperty>(const std::string&)> grabber;
	};

	std::vector<BlockPropertySlot> m_property_edits;
	std::shared_ptr<GraphicsObjectsManager> m_scene_manager;
	std::shared_ptr<BlockClassesManager> m_classesManager;
	model::BlockId m_block_id;
	
};

}