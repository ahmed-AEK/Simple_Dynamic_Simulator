#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/ScrollView.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
    namespace logging
    {
        struct LogLine
        {
            std::string text;
            int category;
            int priority;
        };
    }
    class LogView: public ScrollViewBase
    {
    public:

        LogView(const WidgetSize& size, TTF_Font* font, node::Widget* parent);
        ~LogView();
        void AddLine(logging::LogLine line);
    protected:
        void OnDraw(SDL::Renderer& renderer) override;
        void OnSetSize(const WidgetSize& size) override;
        void OnPositionRequested(float new_position) override;
    private:
        void DrawOutline(SDL::Renderer& renderer);
        void DrawLines(SDL::Renderer& renderer);
        void DrawLinesDownFrom(SDL::Renderer& renderer, size_t start_idx);
        void DrawLinesUpFrom(SDL::Renderer& renderer, size_t end_idx);
        float GetWidgetLinesHeight() const;

        static constexpr size_t npos{ static_cast<size_t>(- 1)};
        size_t m_first_line_idx{ static_cast<size_t>(-1)};
        
        static constexpr float x_margin = 5;

        class TextLine;
        TTF_Font* m_font{};
        std::vector<TextLine> m_text_lines;
        std::vector<logging::LogLine> m_log_lines;
    };
}