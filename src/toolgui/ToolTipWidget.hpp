#pragma once

#include "SDL_Framework/Utility.hpp"
#include "toolgui/Widget.hpp"

namespace node
{

class ToolTipWidget : public Widget
{
public:
    static std::unique_ptr<ToolTipWidget> Create(TTF_Font* font, std::string text, Widget* parent);
    ToolTipWidget(TTF_Font* font, std::string text, const WidgetSize& size, Widget* parent);
    void OnDraw(SDL::Renderer& renderer) override;
private:
    TextPainter m_painter;
};

}
