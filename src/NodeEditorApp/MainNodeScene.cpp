#include "MainNodeScene.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/ButtonWidget.hpp"
#include "BoxObject.hpp"
#include "ExampleContextMenu.hpp"
#include "NodeGraphicsScene.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/tools/ArrowTool.hpp"
#include "toolgui/SidePanel.hpp"
#include "BlockPallete/BlockPallete.hpp"
#include "toolgui/ToolBar.hpp"
#include "GraphicsScene/ToolButton.hpp"

static void AddInitialNodes_forScene(node::GraphicsScene* gScene)
{
    assert(gScene);
    using namespace node;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();
    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 10,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 200,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 400,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 200,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        sceneModel->AddBlock(model);
    }


    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 400,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        sceneModel->AddBlock(model);
    }
    gScene->SetSceneModel(std::move(sceneModel));
}


void node::MainNodeScene::InitializeTools(node::GraphicsScene* gScene)
{
    assert(gScene);
    m_toolsManager = std::make_shared<ToolsManager>(gScene);
    m_toolsManager->AddTool("A", std::make_unique<ArrowTool>(gScene));
    auto toolbar = std::make_unique<ToolBar>(SDL_Rect{ 0,0,0,0 }, this);
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "A", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "S", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "D", m_toolsManager));
    SetToolBar(std::move(toolbar));
    m_toolsManager->OnChangeTool("A");
}

void node::MainNodeScene::InitializeSidePanel(node::GraphicsScene* gScene)
{
    assert(gScene);
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{ 0,0,300,m_rect.h}, this);

    auto&& pallete_provider = std::make_shared<PalleteProvider>();
    for (int i = 0; i < 5; i++)
    {
        auto&& element = std::make_shared<PalleteElement>();
        element->block.SetBounds({ 0,0,BlockPallete::ElementWidth, BlockPallete::ElementHeight });
        element->block.AddSocket(node::model::BlockSocketModel{
            node::model::BlockSocketModel::SocketType::input, {0,0}, {0,0}
            });
        element->block.AddSocket(node::model::BlockSocketModel{
            node::model::BlockSocketModel::SocketType::output, {0,0}, {0,0}
            });
        element->styler = std::make_shared<BlockStyler>();
        element->styler->PositionNodes(element->block);
        element->block_template = "Add";
        pallete_provider->AddElement(element);

    }

    sidePanel->SetWidget(std::make_unique<BlockPallete>(SDL_Rect{ 0,0,200,200 },
        std::move(pallete_provider), this));
    SetSidePanel(std::move(sidePanel));
}

node::MainNodeScene::MainNodeScene(SDL_Rect rect, node::Application* parent)
:Scene(rect, parent)
{

    using namespace node;
    std::unique_ptr<NodeGraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(m_rect, this);
    m_graphicsScene = static_cast<NodeGraphicsScene*>(gScene.get());

    InitializeSidePanel(gScene.get());

    InitializeTools(gScene.get());


    std::unique_ptr<Widget> remove_BTN = std::make_unique<ButtonWidget>(SDL_Rect{ 50, 100, 200, 50 }, "Remove Node",
        [&, scene = gScene.get()]() {
            auto selections = scene->GetCurrentSelection();
            for (auto& item : selections)
            {
                if (GraphicsObject* object = item.GetObjectPtr())
                {
                    if (ObjectType::node == object->GetObjectType())
                    {
                        //static_cast<BlockObject*>(object)->DisconnectSockets();
                        scene->PopObject(object);
                    }
                }
            }
        }, this);

    AddInitialNodes_forScene(gScene.get());

    AddWidget(std::move(remove_BTN), 0);
    SetgScene(std::move(gScene));
}

node::MainNodeScene::~MainNodeScene() = default;

bool node::MainNodeScene::OnRMBUp(const SDL_Point& p)
{
    if (Scene::OnRMBUp(p))
    {
        return true;
    }
    std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this);
    this->ShowContextMenu(std::move(menu), {p.x, p.y});
    return true;
}

