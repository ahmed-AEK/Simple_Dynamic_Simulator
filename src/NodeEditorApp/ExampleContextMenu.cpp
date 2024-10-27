#include "ExampleContextMenu.hpp"

node::ExampleContextMenu::ExampleContextMenu(node::Widget* parent, TTF_Font* font)
: node::ContextMenu{parent,font}
{
    m_items["hello"] = 1;
    m_items["hi"] = 2;
}