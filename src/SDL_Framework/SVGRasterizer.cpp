#include "SVGRasterizer.hpp"
#include "AssetsManager/AssetsManager.hpp"
#include "lunasvg.h"

node::SVGRasterizer::SVGRasterizer(std::string path, int width, int height)
	:m_path{std::move(path)}, m_width{width}, m_height{ height }
{
}

bool node::SVGRasterizer::Draw(SDL_Renderer* renderer, int x, int y)
{
	if (!m_texture.GetTexture())
	{
        if (!ReCreateTexture(renderer))
        {
            return false;
        }
	}

    SDL_Rect draw_area{ x + (m_width - m_actual_size.x) / 2,y + (m_height - m_actual_size.y) / 2,m_actual_size.x,m_actual_size.y};
    SDL_RenderCopy(renderer, m_texture.GetTexture(), nullptr, &draw_area);
    return true;
}

void node::SVGRasterizer::SetSize(int width, int height)
{
	if (width != m_width || height != m_height)
	{
		m_texture.DropTexture();
	}
	m_width = width;
	m_height = height;
}

std::optional<SDL_FPoint> node::SVGRasterizer::GetSVGSize() const
{
    auto document = lunasvg::Document::loadFromFile(m_path);
    if (!document)
    {
        return std::nullopt;
    }
    return SDL_FPoint{ document->width(), document->height() };
}

void node::SVGRasterizer::SetSVGPath(std::string path)
{
    if (path != m_path)
    {
        m_path = path;
        m_texture.DropTexture();
    }
}


static SDL_Point scaleWithFixedAspectRatio(float width, float height, int desired_width, int desired_height)
{
    double aspect_ratio = static_cast<double>(width)/height;
    int target_width = static_cast<int>(desired_height * aspect_ratio);
    if (target_width > desired_width)
    {
        int target_height = static_cast<int>(desired_width / aspect_ratio);
        return { desired_width, target_height };
    }
    return { target_width, desired_height };
}

bool node::SVGRasterizer::ReCreateTexture(SDL_Renderer* renderer)
{
    AssetsManager assets_manager;
    auto resource = assets_manager.GetResource(m_path);
    if (!resource)
    {
        return false;
    }

    auto document = lunasvg::Document::loadFromData(reinterpret_cast<const char*>(resource->data()), resource->size());
    if (document == nullptr)
    {
        SDL_Log("Failed to load SVG! : %s", m_path.c_str());
        return false;
    }
    m_actual_size = scaleWithFixedAspectRatio(document->width(), document->height(), m_width, m_height);
    auto bitmap = document->renderToBitmap(m_actual_size.x, m_actual_size.y);
    if (bitmap.isNull())
    {
        SDL_Log("Failed to render SVG!");
        return false;
    }

    // ARGB32 to RGBA32
    SDLSurface surface{ SDL_CreateRGBSurfaceFrom(bitmap.data(), bitmap.width(), bitmap.height(), 32, bitmap.stride(), 0xFF0000, 0xFF00, 0xFF, 0xFF000000) };
    SDLTexture texture{ SDL_CreateTextureFromSurface(renderer, surface.get()) };
    m_texture.SetTexture(std::move(texture));
    return true;
}
