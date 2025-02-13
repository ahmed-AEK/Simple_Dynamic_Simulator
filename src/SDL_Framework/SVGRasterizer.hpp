#pragma once

#include <string>
#include "SDL_Framework/Utility.hpp"

namespace node
{

class SVGRasterizer
{
public:
	SVGRasterizer(std::string path, int width, int height);
	bool Draw(SDL_Renderer* renderer, float x, float y);
	void SetSize(int width, int height);
	std::optional<SDL_FPoint>  GetSVGSize() const;
	void SetSVGPath(std::string path);
private:
	bool ReCreateTexture(SDL_Renderer* renderer);

	std::string m_path;
	DroppableTexture m_texture;
	int m_width;
	int m_height;
	SDL_Point m_actual_size{};
};

}