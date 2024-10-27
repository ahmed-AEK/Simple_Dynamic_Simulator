#pragma once

#include "toolgui/ContextMenu.hpp"

namespace node
{
    class ExampleContextMenu: public node::ContextMenu
    {
    public:
        explicit ExampleContextMenu(node::Widget* parent, TTF_Font* font);
    };
}