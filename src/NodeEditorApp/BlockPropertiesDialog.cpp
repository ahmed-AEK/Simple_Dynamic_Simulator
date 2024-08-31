#include "BlockPropertiesDialog.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include "BlockClasses/BlockClassesManager.hpp"


// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

node::BlockPropertiesDialog::BlockPropertiesDialog(const model::BlockModel& block, BlockClassesManager& manager, const SDL_Rect& rect, Scene* parent)
	:Dialog{"Block Properties", rect, parent}
{
	assert(parent);
	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{block.GetClass() + " Block"}, SDL_Rect{ 0,0,100,30 }, parent));
	auto class_ptr = manager.GetBlockClassByName(block.GetClass());
	if (class_ptr)
	{
		auto lines = DialogLabel::SplitToLinesofWidth(std::string{ class_ptr->GetDescription() }, parent->GetApp()->getFont().get(), 500);
		const int line_height = TTF_FontHeight(parent->GetApp()->getFont().get());
		int lines_gap = (lines.size() == 0) ? 0 : DialogLabel::LinesMargin * static_cast<int>(lines.size() - 1);
		AddControl(std::make_unique<DialogLabel>(std::move(lines), SDL_Rect{ 0,0,500, line_height * static_cast<int>(lines.size()) + lines_gap }, parent));
	}

	if (block.GetProperties().size())
	{
		AddControl(std::make_unique<SeparatorControl>(SDL_Rect{ 0,0,500, 2 }, parent));
	}

	for (const auto& property : block.GetProperties())
	{
		std::string initial_value =
			std::visit(
				overloaded{ 
				[](const std::string& item) {return item; },
				[](const auto& other) { return std::to_string(other); }
				},property.prop);
		AddControl(std::make_unique<PropertyEditControl>(property.name, 200, std::move(initial_value), SDL_Rect{0,0,500, 35}, parent));
	}
	AddButton("Ok", [this]() {this->TriggerOk(); });
	AddButton("Cancel", [this]() {this->TriggerClose(); });
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
		if (captured_letters == sv.size())
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

node::DialogLabel::DialogLabel(std::vector<std::string> lines, const SDL_Rect& rect, Scene* parent)
	:DialogControl{rect, parent}, m_lines{std::move(lines)}
{
	assert(parent);
}

void node::DialogLabel::Draw(SDL_Renderer* renderer)
{
	int y = 0;

	int font_height = TTF_FontHeight(GetScene()->GetApp()->getFont().get());
	for (const auto& line : m_lines)
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto textSurface = SDLSurface{ TTF_RenderText_Solid(GetScene()->GetApp()->getFont().get(), line.c_str(), Black) };
		auto textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };

		SDL_Rect text_rect{};
		SDL_QueryTexture(textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
		text_rect.x = GetRect().x;
		text_rect.y = GetRect().y + y;
		SDL_RenderCopy(renderer, textTexture.get(), NULL, &text_rect);
		y += font_height + LinesMargin;
	}

}

node::PropertyEditControl::PropertyEditControl(std::string name, int name_width, std::string initial_value, const SDL_Rect& rect, Scene* parent)
	:DialogControl{ rect, parent }, m_name{ std::move(name) }, m_name_width{ name_width }, m_value{std::move(initial_value)}
{
	assert(parent);
}

void node::PropertyEditControl::Draw(SDL_Renderer* renderer)
{
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto textSurface = SDLSurface{ TTF_RenderText_Solid(GetScene()->GetApp()->getFont().get(), m_name.c_str(), Black) };
		auto textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };

		SDL_Rect text_rect{};
		SDL_QueryTexture(textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
		text_rect.x = GetRect().x;
		text_rect.y = GetRect().y;
		SDL_RenderCopy(renderer, textTexture.get(), NULL, &text_rect);
	}


	SDL_Rect edit_box = GetRect();
	edit_box.x += m_name_width;
	edit_box.w -= m_name_width;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &edit_box);
	SDL_Rect inner_rect{ edit_box };
	inner_rect.x += 2;
	inner_rect.y += 2;
	inner_rect.w -= 4;
	inner_rect.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &inner_rect);
	{
		SDL_Color Black = { 50, 50, 50, 255 };
		auto textSurface = SDLSurface{ TTF_RenderText_Solid(GetScene()->GetApp()->getFont().get(), m_value.c_str(), Black) };
		auto textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };

		SDL_Rect text_rect{};
		SDL_QueryTexture(textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
		text_rect.x = inner_rect.x;
		text_rect.y = inner_rect.y;
		SDL_RenderCopy(renderer, textTexture.get(), NULL, &text_rect);
	}
}

void node::SeparatorControl::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &GetRect());
}
