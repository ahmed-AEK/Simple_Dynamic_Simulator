#pragma once

#include "toolgui/Dialog.hpp"

class SDLFont;

namespace node
{
class BlockClassesManager;

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

class PropertyEditControl : public DialogControl
{
public:
	PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent);
	void Draw(SDL_Renderer* renderer) override;
private:
	std::string m_name;
	int m_name_width;
	std::string m_value;
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
	BlockPropertiesDialog(const model::BlockModel& block, BlockClassesManager& manager, const SDL_Rect& rect, Scene* parent);
private:

};

}