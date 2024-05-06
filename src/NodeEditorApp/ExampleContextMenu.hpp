#pragma once

#include "toolgui/ContextMenu.hpp"

namespace node
{
    class ExampleContextMenu: public node::ContextMenu
    {
    public:
        ExampleContextMenu(node::Scene* parent);
    };
}