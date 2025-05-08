#pragma once

#include <string>
#include "SDL_Framework/Utility.hpp"

namespace node
{

class SVGRasterizer
{
public:
	SVGRasterizer(std::string path, int width, int height);
	bool Draw(SDL_Renderer* renderer, float x, float y, bool is_dark_mode);
	void SetSize(int width, int height);
	std::optional<SDL_FPoint>  GetSVGSize() const;
	void SetSVGPath(std::string path);
private:
	bool ReCreateTexture(SDL_Renderer* renderer);
	void SetDarkMode(bool value);

	std::string m_path;
	DroppableTexture m_texture;
	int16_t m_width;
	int16_t m_height;
	bool m_dark_mode = false;
	SDL_Point m_actual_size{};
};

}