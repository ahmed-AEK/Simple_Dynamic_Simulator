#pragma once

#include "toolgui/Dialog.hpp"

class SDLFont;

namespace node
{
class BlockClassesManager;
class SceneModelManager;

class DialogLabel: public DialogControl
{
public:
	DialogLabel(std::vector<std::string> lines, const SDL_Rect& rect, Scene* parent);
	static std::vector<std::string> SplitToLinesofWidth(const std::string& str, TTF_Font* font, int width);
	void Draw(SDL_Renderer* renderer) override;

	static constexpr int LinesMargin = 5;
private:
	std::vector<std::string> m_lines;
};

class LineEditControl : public Widget
{
public:
	LineEditControl(std::string initial_value, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_value; }
protected:
	void OnChar(int32_t key) override;
	void OnKeyPress(int32_t key) override;
private:
	std::string m_value;
};
class PropertyEditControl : public DialogControl
{
public:
	PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
	const std::string& GetValue() const { return m_edit.GetValue(); }
protected:
	void OnSetRect(const SDL_Rect& rect) override;
	Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
	
private:
	LineEditControl m_edit;
	std::string m_name;
	int m_name_width;
};

class SeparatorControl : public DialogControl
{
public:
	using DialogControl::DialogControl;
	void Draw(SDL_Renderer* renderer) override;
};


class BlockPropertiesDialog: public Dialog
{
public:
	BlockPropertiesDialog(const model::BlockModel& block, std::shared_ptr<SceneModelManager> SceneModel, BlockClassesManager& manager, const SDL_Rect& rect, Scene* parent);
protected:
	void OnOk() override;
private:
	struct BlockPropertySlot
	{
		PropertyEditControl* property_edit;
		std::function<std::optional<model::BlockProperty>(const std::string&)> grabber;
	};

	std::vector<BlockPropertySlot> m_property_edits;
	std::shared_ptr<SceneModelManager> m_model_manager;
	model::BlockId m_block_id;
	
};

}