#include "MainNodeScene.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/ButtonWidget.hpp"
#include "toolgui/SidePanel.hpp"
#include "toolgui/ToolBar.hpp"

#include "BoxObject.hpp"
#include "ExampleContextMenu.hpp"
#include "NodeGraphicsScene.hpp"

#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/tools/DeleteTool.hpp"
#include "GraphicsScene/ToolButton.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"

#include "BlockPallete/BlockPallete.hpp"

static void AddInitialNodes_forScene(node::GraphicsObjectsManager* manager)
{
    assert(manager);
    using namespace node;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();
    {
        auto model = std::make_shared<node::model::BlockModel>(model::BlockId{1}, model::Rect{ 10,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{1} });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(model::BlockId{2}, model::Rect{ 200,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(model::BlockId{3}, model::Rect{ 400,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        sceneModel->AddBlock(model);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(model::BlockId{4}, model::Rect{ 200,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        sceneModel->AddBlock(model);
    }


    {
        auto model = std::make_shared<node::model::BlockModel>(model::BlockId{5}, model::Rect{ 400,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        sceneModel->AddBlock(model);
    }
    manager->SetSceneModel(std::make_shared<SceneModelManager>(std::move(sceneModel)));
}


void node::MainNodeScene::InitializeTools()
{
    auto toolbar = std::make_unique<ToolBar>(SDL_Rect{ 0,0,0,0 }, this);
    m_toolsManager = std::make_shared<ToolsManager>(m_graphicsScene, toolbar.get());
    m_toolsManager->AddTool("A", std::make_shared<ArrowTool>(m_graphicsScene, m_graphicsObjectsManager.get()));
    m_toolsManager->AddTool("D", std::make_shared<DeleteTool>(m_graphicsScene, m_graphicsObjectsManager.get()));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "A", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "S", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "D", m_toolsManager));
    SetToolBar(std::move(toolbar));
    m_toolsManager->ChangeTool("A");
}

void node::MainNodeScene::InitializeSidePanel(node::GraphicsScene* gScene)
{
    assert(gScene);
    UNUSED_PARAM(gScene);
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{ 0,0,300,m_rect.h}, this);

    auto&& pallete_provider = std::make_shared<PalleteProvider>();
    for (int i = 0; i < 5; i++)
    {
        auto&& element = std::make_shared<PalleteElement>();
        element->block.SetBounds({ 0,0,BlockPallete::ElementWidth, BlockPallete::ElementHeight });
        element->block.AddSocket(node::model::BlockSocketModel{
            node::model::BlockSocketModel::SocketType::input, model::SocketId{0}, {0,0}
            });
        element->block.AddSocket(node::model::BlockSocketModel{
            node::model::BlockSocketModel::SocketType::output, model::SocketId{1}, {0,0}
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
    m_graphicsObjectsManager = std::make_unique<GraphicsObjectsManager>(*gScene);
    m_graphicsScene->Attach(*m_graphicsObjectsManager);

    InitializeSidePanel(gScene.get());

    InitializeTools();

    AddInitialNodes_forScene(m_graphicsObjectsManager.get());

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

