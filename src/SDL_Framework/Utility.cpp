#include "SDL_Framework/Utility.hpp"

#include <mutex>
#include <unordered_map>
#include <array>
#include <cassert>
#include <utility>
#include <cmath>

#include "boost/container_hash/hash.hpp"

template <typename Key, typename Value>
class CacheStore
{
    struct ValueStore
    {
        std::weak_ptr<Value> ptr;
        Value val;
    };
    struct Deleter
    {
        std::unordered_map<Key, ValueStore>& map_ref;
        Key key;
        void operator()(Value*)
        {
            map_ref.erase(key);
        }
    };
public:
    CacheStore() = default;
    CacheStore(const CacheStore&) = delete;
    CacheStore& operator=(const CacheStore&) = delete;

    template <typename K, typename U>
    std::shared_ptr<Value> AddItem(K&& key, U&& value)
    {

        std::shared_ptr<Value> result_obj;
        auto result = m_stored_values.emplace(key, ValueStore{ std::weak_ptr<Value>{}, std::forward<U>(value) });
        if (!result.second)
        {
            result_obj = result.first->second.ptr.lock();
            return result_obj;
        }
        result_obj = std::shared_ptr<Value>{ &(result.first->second.val), Deleter{m_stored_values, key} };
        result.first->second.ptr = result_obj;
        return result_obj;
    }
    template <typename U>
    std::shared_ptr<Value> GetItem(U&& key)
    {
        auto it = m_stored_values.find(std::forward<U>(key));
        if (it != m_stored_values.end())
        {
            return it->second.ptr.lock();
        }
        return nullptr;
    }
private:
    std::unordered_map<Key, ValueStore> m_stored_values;
};

static bool operator==(const SDL_Color& r1, const SDL_Color& r2)
{
    return r1.r == r2.r && r1.g == r2.g && r1.b == r2.b;
}

struct RoundRectSpec
{
    SDL_Color color;
    int radius;
    bool operator==(const RoundRectSpec&) const = default;
};

template <>
struct std::hash<RoundRectSpec>
{
    std::size_t operator()(const RoundRectSpec& k) const
    {
        size_t hash = 0;
        boost::hash_combine(hash, k.color.r);
        boost::hash_combine(hash, k.color.g);
        boost::hash_combine(hash, k.color.b);
        boost::hash_combine(hash, k.radius);
        return hash;
    }
};

CacheStore<RoundRectSpec, DroppableTexture> g_roundrect_store;

void FilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& rect, int radius, const SDL_Color& color)
{
    DrawRoundRect(renderer, rect, radius, color);
}

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& original_rect, int original_radius, float thickness, const SDL_Color& color1, const SDL_Color& color2)
{
    {
        SDL_FRect rect = original_rect;
        int radius = original_radius;
        FilledRoundRect(renderer, rect, radius, color1);
    }

    {
        SDL_FRect rect;
        rect.x = original_rect.x + thickness;
        rect.y = original_rect.y + thickness;
        rect.w = original_rect.w - 2 * thickness;
        rect.h = original_rect.h - 2 * thickness;
        int radius = static_cast<int>(original_radius - thickness);
        FilledRoundRect(renderer, rect, radius, color2);
    }
}



void DrawFilledArcAA3(SDL_Renderer* renderer, int radius, const SDL_Color color);

std::mutex painters_mutex{};

std::unordered_map<SDL_Texture*, DroppableTexture*> Texturepainters{};


void textures::ResetAllTextures()
{
    std::lock_guard g{ painters_mutex };
    for (auto&& painter_it : Texturepainters)
    {
        painter_it.second->DropTextureNoLock();
    }
    Texturepainters.clear();
}



void RoundRectPainter::Draw(SDL_Renderer* renderer, SDL_FRect rect, int radius, const SDL_Color& color)
{
    if (!m_arc_texture || !m_arc_texture->GetTexture() || radius != stored_radius || color.r != stored_color.r || color.g != stored_color.g || color.b != stored_color.b)
    {
        stored_color = color;
        stored_radius = radius;
        ReCreateArcTexture(renderer);
    }
    rect.x = std::floor(rect.x);
    rect.y = std::floor(rect.y);
    rect.h = std::ceil(rect.h);
    rect.w = std::ceil(rect.w);

    float radius_f = static_cast<float>(radius);
    {
        // draw rects
        SDL_SetRenderDrawColor(renderer, stored_color.r, stored_color.g, stored_color.b, 255);
        SDL_FRect rects[]{
            {rect.x + radius_f, rect.y, rect.w - 2 * radius_f, radius_f},
            {rect.x, rect.y + radius_f, rect.w, rect.h - 2 * radius_f},
            {rect.x + radius_f, rect.y + rect.h - radius_f, rect.w - 2 * radius_f, radius_f},
        };

        SDL_RenderFillRects(renderer, rects, static_cast<int>(std::size(rects)));
    }

    {
        // draw lower right corner
        SDL_FRect trgt{ rect.x + rect.w - radius_f, rect.y + rect.h - radius_f, radius_f, radius_f };
        if (m_draw_sides[2])
        {
            SDL_RenderTextureRotated(renderer, m_arc_texture->GetTexture(), 0, &trgt, 0, 0, SDL_FLIP_NONE);
        }
        else
        {
            SDL_RenderFillRect(renderer, &trgt);
        }
    }
    {
        // draw upper right corner
        SDL_FRect trgt{ rect.x + rect.w - radius_f, rect.y, radius_f, radius_f };
        if (m_draw_sides[1])
        {
            SDL_RenderTextureRotated(renderer, m_arc_texture->GetTexture(), 0, &trgt, 270, 0, SDL_FLIP_NONE);
        }
        else
        {
            SDL_RenderFillRect(renderer, &trgt);
        }
    }
    {
        // draw upper left corner
        SDL_FRect trgt{ rect.x, rect.y, radius_f, radius_f };
        if (m_draw_sides[0])
        {
            SDL_RenderTextureRotated(renderer, m_arc_texture->GetTexture(), 0, &trgt, 180, 0, SDL_FLIP_NONE);
        }
        else
        {
            SDL_RenderFillRect(renderer, &trgt);
        }
    }
    {
        // draw lower left corner
        SDL_FRect trgt{ rect.x, rect.y + rect.h - radius_f, radius_f, radius_f };
        if (m_draw_sides[3])
        {
            SDL_RenderTextureRotated(renderer, m_arc_texture->GetTexture(), 0, &trgt, 90, 0, SDL_FLIP_NONE);
        }
        else
        {
            SDL_RenderFillRect(renderer, &trgt);
        }
    }
}

void RoundRectPainter::SetDrawSides(bool NW, bool NE, bool SE, bool SW)
{
    m_draw_sides = std::array<bool,4>{ NW, NE, SE, SW };
}

void RoundRectPainter::ReCreateArcTexture(SDL_Renderer* renderer)
{
    RoundRectSpec spec{ stored_color,stored_radius };
    if (auto ptr = g_roundrect_store.GetItem(spec))
    {
        m_arc_texture = std::move(ptr);
    }
    else
    {
        m_arc_texture = g_roundrect_store.AddItem(spec, DroppableTexture{});
    }

    if (!m_arc_texture->GetTexture())
    {
        SDL_Texture* old_texture = SDL_GetRenderTarget(renderer);
        {
            SDLTexture stored_arc_texture{ SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, stored_radius, stored_radius) };
            m_arc_texture->SetTexture(std::move(stored_arc_texture));
        }

        SDL_SetTextureBlendMode(m_arc_texture->GetTexture(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, m_arc_texture->GetTexture());
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); // set color to transparent white
        SDL_RenderClear(renderer); // clear renderer
        DrawFilledArcAA3(renderer, stored_radius - 1, stored_color);
        SDL_SetRenderTarget(renderer, old_texture);
    }
}

void DrawFilledArcAA3(SDL_Renderer* renderer, int radius, const SDL_Color color)
{
    using std::floor;
    float y = 0;
    float max_y = static_cast<float>(radius);
    float raidus_squared = static_cast<float>(radius * radius);
    while (y <= max_y)
    {
        double x = sqrt(raidus_squared - y * y);
        if (std::isnan(x))
        {
            break;
        }
        {
            float max_x = static_cast<float>(floor(x));
            if (floor(x) + 2 < y)
            {
                break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            std::array<SDL_FPoint, 4> points{
            SDL_FPoint{0, y}, SDL_FPoint{max_x,y},
            SDL_FPoint{y, 0}, SDL_FPoint{y, max_x},
            };
            SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));
        }

        {
            double overflow = x - floor(x);
            int color_value = static_cast<int>(255 * overflow);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, static_cast<Uint8>(color_value));
            float max_x = static_cast<float>(floor(x + 1));
            std::array<SDL_FPoint, 2> points{
                SDL_FPoint{max_x, y},
                SDL_FPoint{y, max_x},
            };

            SDL_RenderPoints(renderer, points.data(), static_cast<int>(points.size()));
        }

        y++;
    }
}



enum class ArcDirection
{
    pos = 1,
    neg = -1,
};


template <ArcDirection x_mul, ArcDirection y_mul>
void DrawFilledArcAA4(SDL_Renderer* renderer, SDL_FPoint midpoint, float radius, const SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const int x_mul_i = static_cast<int>(x_mul);
    const int y_mul_i = static_cast<int>(y_mul);
    float y = 0;
    float max_y = radius;
    float raidus_squared = radius * radius;
    while (y <= max_y)
    {
        float x = sqrt(raidus_squared - y * y);
        if (std::isnan(x))
        {
            break;
        }
        {
            float max_x = static_cast<float>(floor(x));
            if (floor(x) + 2 < y)
            {
                break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            std::array<SDL_FPoint, 4> points{
            SDL_FPoint{midpoint.x, midpoint.y + y * y_mul_i}, SDL_FPoint{midpoint.x + max_x * x_mul_i, midpoint.y + y * y_mul_i},
            SDL_FPoint{midpoint.x + y * x_mul_i, midpoint.y}, SDL_FPoint{midpoint.x + y * x_mul_i, midpoint.y + max_x * y_mul_i},
            };
            SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));
        }

        {
            float overflow = x - floor(x);
            int color_value = static_cast<int>(255 * overflow);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, static_cast<Uint8>(color_value));
            float max_x = static_cast<float>(floor(x + 1));
            std::array<SDL_FPoint, 2> points{
                SDL_FPoint{midpoint.x + max_x * x_mul_i, midpoint.y + y * y_mul_i},
                SDL_FPoint{midpoint.x + y * x_mul_i, midpoint.y + max_x * y_mul_i},
            };

            SDL_RenderPoints(renderer, points.data(), static_cast<int>(points.size()));
        }

        y++;
    }
}

void DrawRoundRect(SDL_Renderer* renderer, const SDL_FRect& rect, int radius, const SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    float radius_f = static_cast<float>(radius);
    SDL_FRect rects[]{
        {rect.x + radius_f, rect.y, rect.w - 2 * radius_f, radius_f},
        {rect.x, rect.y + radius_f, rect.w, rect.h - 2 * radius_f},
        {rect.x + radius_f, rect.y + rect.h - radius_f, rect.w - 2 * radius_f, radius_f},
    };

    SDL_RenderFillRects(renderer, rects, static_cast<int>(std::size(rects)));

    DrawFilledArcAA4<ArcDirection::pos, ArcDirection::pos>(renderer, { rect.x + rect.w - radius_f, rect.y + rect.h - radius_f }, radius_f - 1, color);
    DrawFilledArcAA4<ArcDirection::neg, ArcDirection::pos>(renderer, { rect.x + radius_f - 1, rect.y + rect.h - radius_f }, radius_f - 1, color);
    DrawFilledArcAA4<ArcDirection::neg, ArcDirection::neg>(renderer, { rect.x + radius_f - 1, rect.y + radius_f - 1 }, radius_f - 1, color);
    DrawFilledArcAA4<ArcDirection::pos, ArcDirection::neg>(renderer, { rect.x + rect.w - radius_f, rect.y + radius_f - 1 }, radius_f - 1, color);

}

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2, RoundRectPainter& outer, RoundRectPainter& inner)
{
    {
        SDL_FRect rect = original_rect;
        int radius = original_radius;
        outer.Draw(renderer, rect, radius, color1);
    }

    {
        SDL_FRect rect;
        rect.x = original_rect.x + thickness;
        rect.y = original_rect.y + thickness;
        rect.w = original_rect.w - 2 * thickness;
        rect.h = original_rect.h - 2 * thickness;
        int radius = original_radius - thickness;
        inner.Draw(renderer, rect, radius, color2);
    }
}

void TextPainter::Draw(SDL_Renderer* renderer, const SDL_FPoint point, const SDL_Color color)
{
    if (!m_text.size())
    {
        return;
    }
    assert(m_font);
    AssureTexture(renderer, color);

    SDL_FRect text_rect{ 0,0,0,0 };
    SDL_GetTextureSize(m_text_texture.GetTexture(), &text_rect.w, &text_rect.h);
    text_rect.x = std::floor(point.x);
    text_rect.y = std::floor(point.y);
    SDL_RenderTexture(renderer, m_text_texture.GetTexture(), NULL, &text_rect);
}

SDL_FRect TextPainter::GetRect(SDL_Renderer* renderer, const SDL_Color color)
{
    assert(m_font);
    AssureTexture(renderer, color);
    SDL_FRect text_rect{ 0,0,0,0 };
    SDL_GetTextureSize(m_text_texture.GetTexture(), &text_rect.w, &text_rect.h);
    return text_rect;
}

void TextPainter::SetText(std::string_view text)
{
    if (text != m_text)
    {
        m_text = std::string{ text };
        m_text_texture.DropTexture();
    }
}

void TextPainter::ReCreateTexture(SDL_Renderer* renderer)
{
    SDL_Color color = m_stored_color;
    auto textSurface = SDLSurface{ TTF_RenderText_Blended(m_font, m_text.c_str(), m_text.size(), color)};
    auto texture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };
    m_text_texture.SetTexture(std::move(texture));
    SDL_FRect text_rect{};
    SDL_GetTextureSize(m_text_texture.GetTexture(), &text_rect.w, &text_rect.h);
}

void TextPainter::AssureTexture(SDL_Renderer* renderer, const SDL_Color& color)
{
    assert(m_font);
    if (!m_text_texture.GetTexture() || color.r != m_stored_color.r || color.g != m_stored_color.g || color.b != m_stored_color.b)
    {
        m_stored_color = color;
        m_text_texture.DropTexture();
        ReCreateTexture(renderer);
    }
}

void TextPainter::SetFont(TTF_Font* font)
{
    m_font = font;
    m_text_texture.DropTexture();
}

DroppableTexture::DroppableTexture(SDLTexture&& texture)
{
    SetTexture(std::move(texture));
}

DroppableTexture::DroppableTexture(DroppableTexture&& other) noexcept
{
    m_stored_texture = std::move(other.m_stored_texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        auto it = Texturepainters.find(m_stored_texture.get());
        it->second = this;
    }
}

DroppableTexture& DroppableTexture::operator=(DroppableTexture&& other) noexcept
{
    if (&other == this)
    {
        return *this;
    }
    DropTexture();
    m_stored_texture = std::move(other.m_stored_texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        auto it = Texturepainters.find(m_stored_texture.get());
        it->second = this;
    }
    return *this;
}

DroppableTexture::~DroppableTexture()
{
    DropTexture();
}

void DroppableTexture::DropTexture()
{
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        Texturepainters.erase(m_stored_texture.get());   
    }
    m_stored_texture = nullptr;
}

void DroppableTexture::DropTextureNoLock()
{
    m_stored_texture = nullptr;
}

void DroppableTexture::SetTexture(SDLTexture texture)
{
    DropTexture();
    m_stored_texture = std::move(texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        Texturepainters.emplace(m_stored_texture.get(), this);
    }
}

SDL_Texture* DroppableTexture::GetTexture()
{
    return m_stored_texture.get();
}


TruncatedTextPainter::TruncatedTextPainter(TTF_Font* font)
    :m_painter{font}
{

}

void TruncatedTextPainter::Draw(SDL::Renderer& renderer, const SDL_FPoint point, const SDL_Color color)
{
    if (m_dirty)
    {
        UpdatePainter();
    }

    m_painter.Draw(renderer, { point.x - static_cast<float>(m_local_offset), point.y }, color);
}

int TruncatedTextPainter::GetHeight() const
{
    return TTF_GetFontHeight(m_painter.GetFont());
}

void TruncatedTextPainter::SetPixelOffset(size_t offset)
{
    if (m_pixel_offset != offset)
    {
        m_dirty = true;
    }
    m_pixel_offset = offset;
}

void TruncatedTextPainter::SetWidth(size_t width)
{
    if (m_width != static_cast<int>(width))
    {
        m_dirty = true;
    }
    m_width = static_cast<int>(width);
}

void TruncatedTextPainter::SetText(std::string text)
{
    m_text = std::move(text);
    m_dirty = true;
}
TTF_Font* TruncatedTextPainter::GetFont() const
{
    return m_painter.GetFont();
}
void TruncatedTextPainter::UpdatePainter()
{
    assert(m_painter.GetFont());
    m_dirty = false;
    int measured_width = 0;
    size_t measured_length = 0;
    if (m_pixel_offset == 0)
    {
        m_local_offset = 0;
        measured_length = 0;
        measured_width = 0;
    }
    else
    {
        TTF_MeasureString(m_painter.GetFont(), m_text.c_str(), m_text.size(), static_cast<int>(m_pixel_offset), &measured_width, &measured_length);
        m_local_offset = static_cast<uint16_t>(m_pixel_offset - measured_width);
        if (measured_length == m_text.size())
        {
            m_painter.SetText("");
            return;
        }
    }
    std::string_view rest_of_text = std::string_view{ m_text }.substr(measured_length);
    if (TTF_MeasureString(m_painter.GetFont(), rest_of_text.data(), rest_of_text.size(), m_width, &measured_width, &measured_length))
    {
        if (measured_length != rest_of_text.size())
        {
            measured_length++;
        }
        if (measured_length != rest_of_text.size())
        {
            measured_length++;
        }
        m_painter.SetText(std::string{ rest_of_text.begin(), rest_of_text.begin() + measured_length });

    }
    
}