#include "MainNodeScene.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/ButtonWidget.hpp"
#include "toolgui/SidePanel.hpp"
#include "toolgui/ToolBar.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/Dialog.hpp"

#include "ExampleContextMenu.hpp"
#include "NodeGraphicsScene.hpp"

#include "NodeModels/NodeScene.hpp"

#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/tools/DeleteTool.hpp"
#include "GraphicsScene/tools/NetTool.hpp"
#include "GraphicsScene/ToolButton.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"

#include "BlockClasses/BlockClassesManager.hpp"
#include "BlockClasses/GainBlockClass.hpp"
#include "BlockClasses/ConstantSourceClass.hpp"
#include "BlockClasses/ScopeDisplayClass.hpp"
#include "BlockClasses/RampSourceClass.hpp"
#include "BlockClasses/IntegrationBlockClass.hpp"
#include "BlockClasses/DerivativeBlockClass.hpp"

#include "BlockPallete/BlockPallete.hpp"

#include "NodeEditorApp/SimulatorRunner.hpp"
#include "NodeEditorApp/BlockPropertiesDialog.hpp"

static void AddInitialNodes_forScene(node::GraphicsObjectsManager* manager)
{
    assert(manager);
    using namespace node;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();
    {
        model::BlockModel model{ model::BlockId{ 1 }, model::Rect{ 10,10,100,100 }};
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetClass("Gain");
        model.GetProperties().push_back(model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }

    {
        model::BlockModel model{ model::BlockId{2}, model::Rect{ 200,10,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetClass("Gain");
        model.GetProperties().push_back(model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }

    {
        model::BlockModel model{ model::BlockId{3}, model::Rect{ 400,10,100,100 }};
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetClass("Gain");
        model.GetProperties().push_back(model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }

    {
        model::BlockModel model{ model::BlockId{4}, model::Rect{ 200,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetClass("Gain");
        model.GetProperties().push_back(model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }


    {
        model::BlockModel model{ model::BlockId{5}, model::Rect{ 400,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.SetClass("Scope Display");
        model.GetProperties().push_back(model::BlockProperty{ "Inputs",model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1) });
        sceneModel->AddBlock(std::move(model));
    }

    manager->SetSceneModel(std::make_shared<SceneModelManager>(std::move(sceneModel)));
}


void node::MainNodeScene::RunSimulator()
{
    if (!m_current_running_simulator)
    {
        auto runner = std::make_shared<SimulatorRunner>(
            m_graphicsObjectsManager->GetSceneModel()->GetModel(),
            m_classesManager,
            std::function<void()>{
            [this] { GetApp()->AddMainThreadTask([this]() { this->CheckSimulatorEnded(); });} 
        });
        m_current_running_simulator = runner;
        runner->Run();
    }
}

void node::MainNodeScene::StopSimulator()
{
    if (m_current_running_simulator)
    {
        m_current_running_simulator->Stop();
    }
}

void node::MainNodeScene::CheckSimulatorEnded()
{
    if (m_current_running_simulator && m_current_running_simulator->IsEnded())
    {
        if (auto result = m_current_running_simulator->GetResult())
        {
            OnSimulationEnd(*result);
        }
        else
        {
            assert(false); // simulation ended but no result !!
        }
        
    }
}

void node::MainNodeScene::DeleteEventReceiver(NodeSceneEventReceiver* handler)
{
    auto it = std::find_if(m_event_receivers.begin(), m_event_receivers.end(), [&](const auto& ptr) {return ptr.get() == handler; });
    assert(it != m_event_receivers.end());
    if (it != m_event_receivers.end())
    {
        m_event_receivers.erase(it);
    }
}

namespace
{
struct DoubleClickEventReceiver : public node::NodeSceneEventReceiver, public node::SingleObserver<node::BlockDoubleClickedEvent>
{
    DoubleClickEventReceiver(node::MainNodeScene& scene, std::function<void(node::BlockObject&)> functor)
        :NodeSceneEventReceiver{ scene }, m_functor{std::move(functor)} {}
    void OnNotify(node::BlockDoubleClickedEvent& block) override
    {
        m_functor(*(block.block));
    }
    std::function<void(node::BlockObject&)> m_functor;
};

}

void node::MainNodeScene::InitializeTools()
{
    auto toolbar = std::make_unique<ToolBar>(SDL_Rect{ 0,0,0,0 }, this);
    m_toolsManager = std::make_shared<ToolsManager>(m_graphicsScene, toolbar.get());
    {
        auto arrow_tool = std::make_shared<ArrowTool>(m_graphicsScene, m_graphicsObjectsManager.get());
        auto receiver = std::make_unique<DoubleClickEventReceiver>(*this, [this](BlockObject& block) {this->OpenBlockDialog(block); } );
        arrow_tool->Attach(*receiver);
        AddEventReceiver(std::move(receiver));
        m_toolsManager->AddTool("A", std::move(arrow_tool));
    }
    m_toolsManager->AddTool("D", std::make_shared<DeleteTool>(m_graphicsScene, m_graphicsObjectsManager.get()));
    m_toolsManager->AddTool("N", std::make_shared<NetTool>(m_graphicsScene, m_graphicsObjectsManager.get()));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "A", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "N", m_toolsManager));
    toolbar->AddButton(std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "D", m_toolsManager));
    toolbar->AddSeparator();
    toolbar->AddButton(std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "P", [this]() {SDL_Log("Properties!"); this->OpenPropertiesDialog(); }));
    toolbar->AddButton(std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "R", [this]() {SDL_Log("Run!"); this->RunSimulator(); }, 
        [this]() { return this->m_current_running_simulator != nullptr; }));
    toolbar->AddButton(std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "S", [this]() {SDL_Log("Stop!"); this->StopSimulator(); }, 
        [this]() { return this->m_current_running_simulator == nullptr; }));
    SetToolBar(std::move(toolbar));
    m_toolsManager->ChangeTool("A");
}

void node::MainNodeScene::InitializeSidePanel(node::GraphicsScene* gScene)
{
    assert(gScene);
    UNUSED_PARAM(gScene);
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{ 0,0,300,m_rect.h}, this);

    auto&& pallete_provider = std::make_shared<PalleteProvider>(m_classesManager);
    auto block_template = BlockTemplate{
        "Gain1",
        "Gain",
        "Default",
        std::vector<model::BlockProperty>{
            model::BlockProperty{"Multiplier", model::BlockPropertyType::FloatNumber, 1.0}
        }
    };


    for (int i = 0; i < 1; i++)
    {
        pallete_provider->AddElement(block_template);
    }

    auto integrate_block = BlockTemplate{
        "Integration",
        "Integration",
        "Default",
        std::vector<model::BlockProperty>{
        }
    };
    pallete_provider->AddElement(std::move(integrate_block));

    auto deriv_block = BlockTemplate{
    "Derivative",
    "Derivative",
    "Default",
    std::vector<model::BlockProperty>{
    }
    };
    pallete_provider->AddElement(std::move(deriv_block));

    auto constant_block = BlockTemplate{
        "Constant Source",
        "Constant Source",
        "Default",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Value", model::BlockPropertyType::FloatNumber, 1.0}
        }
    };
    pallete_provider->AddElement(std::move(constant_block));

    auto ramp_block = BlockTemplate{
        "Ramp",
        "Ramp",
        "Default",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Slope", model::BlockPropertyType::FloatNumber, 1.0}
        }
    };
    pallete_provider->AddElement(std::move(ramp_block));

    auto scope_block = BlockTemplate{
        "Scope",
        "Scope Display",
        "Default",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Inputs", model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1)}
        } 
    };
    pallete_provider->AddElement(std::move(scope_block));


    sidePanel->SetWidget(std::make_unique<BlockPallete>(SDL_Rect{ 0,0,200,200 },
        std::move(pallete_provider), this));
    SetSidePanel(std::move(sidePanel));
}

void node::MainNodeScene::OpenPropertiesDialog()
{
    auto&& selection = m_graphicsScene->GetCurrentSelection();
    if (selection.size() != 1)
    {
        SDL_Log("More than one object selected!");
        return;
    }

    auto object = selection[0].GetObjectPtr();
    if (!object || object->GetObjectType() != ObjectType::block)
    {
        SDL_Log("block not selected!");
        return;
    }

    {
        auto it = m_objects_dialogs.find(static_cast<BlockObject*>(object));
        if (it != m_objects_dialogs.end() && it->second.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.GetObjectPtr());
            BumpDialogToTop(dialog);
            return;
        }
    }

    auto model_id = static_cast<BlockObject*>(object)->GetModelId();
    assert(model_id);
    if (!model_id)
    {   
        SDL_Log("Block has no model!");
        return;
    }

    auto block = m_graphicsObjectsManager->GetSceneModel()->GetModel().GetBlockById(*model_id);
    if (!block)
    {
        SDL_Log("couldn't find the block model!");
        return;
    }

    assert(m_classesManager);
    auto dialog = std::make_unique<BlockPropertiesDialog>(*block, m_graphicsObjectsManager->GetSceneModel(), *m_classesManager, SDL_Rect{ 100,100,300,300 }, this);
    m_objects_dialogs[static_cast<BlockObject*>(object)] = dialog->GetMIHandlePtr();
    AddNormalDialog(std::move(dialog));

}

void node::MainNodeScene::OpenBlockDialog(node::BlockObject& block)
{
    if (!block.GetModelId())
    {
        assert(false);
        return;
    }

    {
        auto it = m_objects_dialogs.find(&block);
        if (it != m_objects_dialogs.end() && it->second.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.GetObjectPtr());
            BumpDialogToTop(dialog);
            return;
        }
    }


    assert(block.GetModelId());
    auto block_model = m_graphicsObjectsManager->GetSceneModel()->GetModel().GetBlockById(*block.GetModelId());
    if (!block_model)
    {
        assert(false);
        return;
    }

    auto class_ptr = m_classesManager->GetBlockClassByName(block_model->get().GetClass());
    if (!class_ptr)
    {
        SDL_Log("class '%s' not found!", block_model->get().GetClass().c_str());
        return;
    }
    if (class_ptr->HasBlockDialog())
    {
        auto sim_data = std::any{};
        {
            auto model_id = *block.GetModelId();
            auto block_it = std::find_if(m_last_simulation_result.begin(), m_last_simulation_result.end(),
                [&](const BlockResult& r) {return r.id == model_id; });
            if (block_it != m_last_simulation_result.end())
            {
                sim_data = block_it->data;
            }
        }
        auto dialog = class_ptr->CreateBlockDialog(*this, *block_model, sim_data);
        if (dialog)
        {
            m_objects_dialogs[&block] = dialog->GetMIHandlePtr();
            AddNormalDialog(std::move(dialog));
        }
    }
}

node::MainNodeScene::MainNodeScene(SDL_Rect rect, node::Application* parent)
:Scene(rect, parent)
{
}

void node::MainNodeScene::OnInit()
{
    using namespace node;
    std::unique_ptr<NodeGraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(m_rect, this);
    m_graphicsScene = static_cast<NodeGraphicsScene*>(gScene.get());
    m_graphicsObjectsManager = std::make_unique<GraphicsObjectsManager>(*gScene);
    m_graphicsScene->Attach(*m_graphicsObjectsManager);

    m_classesManager = std::make_shared<BlockClassesManager>();
    m_classesManager->RegisterBlockClass(std::make_shared<GainBlockClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<ConstantSourceClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<ScopeDisplayClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<RampSourceClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<IntegrationBlockClass>());    
    m_classesManager->RegisterBlockClass(std::make_shared<DerivativeBlockClass>());

    InitializeSidePanel(gScene.get());

    InitializeTools();

    AddInitialNodes_forScene(m_graphicsObjectsManager.get());

    SetgScene(std::move(gScene));
}

node::MainNodeScene::~MainNodeScene() = default;

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void node::MainNodeScene::OnSimulationEnd(SimulationEvent& event)
{
    std::visit(overloaded{
        [](SimulationEvent::NetFloatingError&)
        {
            SDL_Log("Floating Net!");
        },
        [](SimulationEvent::OutputSocketsConflict&)
        {
            SDL_Log("Sockets Conflict!");
        },
        [](SimulationEvent::FloatingInput&)
        {
           SDL_Log("Floating Input!");
        },
        [&](SimulationEvent::Success& e)
        {
            SDL_Log("Success!");
            m_last_simulation_result = std::move(e.result);
        }
        }, event.e);
    
    m_current_running_simulator = nullptr;
    SDL_Log("simulation Ended!");
}

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

