#pragma once

#include "PluginAPI/BlockDialog.hpp"
#include "toolgui/MultiLineEditControl.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include "PluginAPI/BlockClassesManager.hpp"

#include <array>

namespace node
{

class LuaCodeEditor : public DialogControl
{
public:
	LuaCodeEditor(TTF_Font* font, const WidgetSize& size, Dialog* parent);
	void SetText(std::string text);
	std::string GetText() const;
protected:
	void OnSetSize(const WidgetSize& size) override;
private:
	TTF_Font* m_font;
	MultiLineEditControl m_edit;
};

class LuaEditorDialog : public BlockDialog
{
public:
	static std::unique_ptr<LuaEditorDialog> Create(const model::BlockModel& block,
		std::shared_ptr<IBlockPropertiesUpdater> model_updater,
		BlockClassPtr block_class,
		std::string property_name, const WidgetSize& size, Scene* parent);
	LuaEditorDialog(const model::BlockId& block_id,
		std::shared_ptr<IBlockPropertiesUpdater> model_updater,
		BlockClassPtr block_class,
		std::string property_name, std::string code, const WidgetSize& size, Scene* parent);
	void UpdateResults(std::any new_result) override;
protected:
	void OnOk() override;
private:
	bool UpdateProperties();
	LuaCodeEditor* m_editor = nullptr;
	std::shared_ptr<IBlockPropertiesUpdater> m_model_updater;
	BlockClassPtr m_block_class;
	std::string m_property_name;
	model::BlockId m_block_id;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};

}