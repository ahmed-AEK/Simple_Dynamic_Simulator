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


static void AddInitialNodes_forScene(node::GraphicsScene* gScene)
{
    assert(gScene);
    using namespace node;
    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 10,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        auto styler = std::make_shared<node::BlockStyler>();
        styler->PositionNodes(*model);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(gScene, model, styler);
        gScene->AddObject(std::move(obj), 0);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 200,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        auto styler = std::make_shared<node::BlockStyler>();
        styler->PositionNodes(*model);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(gScene, model, styler);
        gScene->AddObject(std::move(obj), 0);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 400,10,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        auto styler = std::make_shared<node::BlockStyler>();
        styler->PositionNodes(*model);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(gScene, model, styler);
        gScene->AddObject(std::move(obj), 0);
    }

    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 200,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        auto styler = std::make_shared<node::BlockStyler>();
        styler->PositionNodes(*model);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(gScene, model, styler);
        gScene->AddObject(std::move(obj), 0);
    }


    {
        auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 400,210,100,100 });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
        model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
        auto styler = std::make_shared<node::BlockStyler>();
        styler->PositionNodes(*model);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(gScene, model, styler);
        gScene->AddObject(std::move(obj), 0);
    }
}

node::MainNodeScene::MainNodeScene(SDL_Rect rect, node::Application* parent)
:Scene(rect, parent)
{

    using namespace node;
    std::unique_ptr<GraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(m_rect, this);

    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{0,0,300,rect.h}, this);
    sidePanel->UpdateWindowSize(rect);
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

    sidePanel->SetWidget(std::make_unique<BlockPallete>(SDL_Rect{0,0,200,200},
        std::move(pallete_provider), this));
    SetSidePanel(std::move(sidePanel));

    gScene->SetTool(std::make_unique<ArrowTool>(gScene.get()));


    m_graphicsScene = static_cast<NodeGraphicsScene*>(gScene.get());
    
    std::unique_ptr<Widget> add_BTN = std::make_unique<ButtonWidget>(50, 50, 200, 50, "Add Node",
        [scene = gScene.get()]() {
            auto model = std::make_shared<node::model::BlockModel>(0, model::Rect{ 10,10,100,100 });
            model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, { model->GetId(),0} });
            model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, { model->GetId(),0 } });
            auto styler = std::make_shared<node::BlockStyler>();
            styler->PositionNodes(*model);
            std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(scene, model, styler);
            scene->AddObject(std::move(obj), 0);
        }, this);
    AddWidget(std::move(add_BTN), 0);

    std::unique_ptr<Widget> remove_BTN = std::make_unique<ButtonWidget>(300, 50, 200, 50, "Remove Node",
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
    AddWidget(std::move(gScene), -1000);


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

