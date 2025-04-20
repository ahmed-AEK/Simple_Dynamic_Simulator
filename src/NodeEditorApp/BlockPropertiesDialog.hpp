#pragma once

#include "toolgui/Dialog.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "PluginAPI/BlockClass.hpp"

class SDLFont;

namespace node
{
class BlockClassesManager;
class GraphicsObjectsManager;
class PropertyEditControl;

class BlockPropertiesDialog: public Dialog
{
public:
	BlockPropertiesDialog( 
		std::shared_ptr<IBlockPropertiesUpdater> updater, 
		BlockClassPtr block_class,
		const model::FunctionalBlockData& block_data,
		const WidgetSize& size, Scene* parent);
protected:
	void OnOk() override;
private:
	struct BlockPropertySlot
	{
		PropertyEditControl* property_edit;
		std::function<std::optional<model::BlockProperty>(const std::string&)> grabber;
	};

	std::shared_ptr<IBlockPropertiesUpdater> m_updater;
	std::vector<BlockPropertySlot> m_property_edits;
	BlockClassPtr m_block_class;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};

}