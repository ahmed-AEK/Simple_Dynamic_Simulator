#include "toolgui/Widget.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include <functional>

namespace node
{
    class ButtonWidget: public Widget
    {
    public:
        ButtonWidget(int x, int y, int width, int height, std::string label, std::function<void(void)> action, node::Scene* parent);
        virtual void Draw(SDL_Renderer* renderer);
    protected:
        virtual MI::ClickEvent OnLMBUp(const SDL_Point& p) override;
    private:
        const int w_margin = 5;
        const int h_margin = 5;
        std::string m_label;
        SDLSurface m_textSurface;
        SDLTexture m_textTexture;
        std::function<void(void)> m_action;
    };
}