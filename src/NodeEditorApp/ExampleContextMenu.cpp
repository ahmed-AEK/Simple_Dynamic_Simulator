#include "ExampleContextMenu.hpp"

node::ExampleContextMenu::ExampleContextMenu(node::Scene* parent)
: node::ContextMenu(parent)
{
    m_items["hello"] = 1;
    m_items["hi"] = 2;
}