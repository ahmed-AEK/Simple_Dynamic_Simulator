#include "NodeEditorApp.hpp"
#include "toolgui/NodeMacros.h"

int main(int argc, char* argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);
    node::NodeEditorApplication App;
    return App.Run();
}