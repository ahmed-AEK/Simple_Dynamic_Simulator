#include "MainNodeScene.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/ButtonWidget.hpp"
#include "BoxObject.hpp"
#include "ExampleContextMenu.hpp"
#include "NodeGraphicsScene.hpp"
#include "GraphicsScene/Node.hpp"
#include "GraphicsScene/NodeSocket.hpp"
#include "GraphicsScene/GraphicsSceneController.hpp"
#include "toolgui/SidePanel.hpp"

node::MainNodeScene::MainNodeScene(SDL_Rect rect, node::Application* parent)
:Scene(rect, parent)
{

    using namespace node;
    std::unique_ptr<GraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(m_rect, this);

    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{0,0,300,rect.h}, this);
    sidePanel->UpdateWindowSize(rect);
    sidePanel->SetWidget(std::make_unique<TestWidget>(SDL_Rect{ 0,0,200,200 }, this));
    SetSidePanel(std::move(sidePanel));

    gScene->SetController(std::make_unique<GraphicsSceneController>(gScene.get()));

    std::unique_ptr<node::Node> obj = std::make_unique<node::Node>(SDL_Rect{10,10,100,100}, gScene.get());
    obj->AddInputSocket(0);
    obj->AddOutputSocket(1);
    gScene->AddObject(std::move(obj), 0);
    m_graphicsScene = static_cast<NodeGraphicsScene*>(gScene.get());
    
    std::unique_ptr<Widget> add_BTN = std::make_unique<ButtonWidget>(50, 50, 200, 50, "Add Node",
        [&, scene = gScene.get()]() {
            std::unique_ptr<node::Node> objx = std::make_unique<node::Node>(SDL_Rect{ 10,10,100,100 }, scene);
            objx->AddInputSocket(0);
            objx->AddOutputSocket(1);
            scene->AddObject(std::move(objx), 5);
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
                        static_cast<Node*>(object)->DisconnectSockets();
                        scene->PopObject(object);
                    }
                }
            }
        }, this);
    AddWidget(std::move(remove_BTN), 0);

    std::unique_ptr<node::Node> obj2 = std::make_unique<node::Node>(SDL_Rect{ 200,10,100,100 }, gScene.get());
    obj2->AddInputSocket(2);
    obj2->AddOutputSocket(3);
    gScene->AddObject(std::move(obj2), 1);

    std::unique_ptr<node::Node> obj3 = std::make_unique<node::Node>(SDL_Rect{ 400,10,100,100 }, gScene.get());
    obj3->AddInputSocket(4);
    obj3->AddOutputSocket(5);
    gScene->AddObject(std::move(obj3), 2);

    std::unique_ptr<node::Node> obj4 = std::make_unique<node::Node>(SDL_Rect{ 200,210,100,100 }, gScene.get());
    obj4->AddInputSocket(6);
    obj4->AddOutputSocket(7);
    gScene->AddObject(std::move(obj4), 3);


    std::unique_ptr<node::Node> obj5 = std::make_unique<node::Node>(SDL_Rect{ 400,210,100,100 }, gScene.get());
    obj5->AddInputSocket(8);
    obj5->AddOutputSocket(9);
    gScene->AddObject(std::move(obj5), 4);

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

