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
#include "GraphicsScene/ToolsManager.hpp"

#include "BlockClasses/BlockClassesManager.hpp"
#include "BlockClasses/GainBlockClass.hpp"
#include "BlockClasses/ConstantSourceClass.hpp"
#include "BlockClasses/ScopeDisplayClass.hpp"
#include "BlockClasses/RampSourceClass.hpp"
#include "BlockClasses/IntegrationBlockClass.hpp"
#include "BlockClasses/DerivativeBlockClass.hpp"
#include "BlockClasses/AddSimpleClass.hpp"
#include "BlockClasses/MultiplyBlockClass.hpp"
#include "BlockClasses/SineSourceClass.hpp"
#include "BlockClasses/StepSourceClass.hpp"

#include "BlockClasses/BlockDialog.hpp"

#include "BlockPallete/BlockPallete.hpp"

#include "NodeSDLStylers/BlockStylerFactory.hpp"
#include "NodeSDLStylers/DefaultBlockStyler.hpp"
#include "NodeSDLStylers/TextBlockStyler.hpp"
#include "NodeSDLStylers/GainBlockStyler.hpp"
#include "NodeSDLStylers/PropertyPrintStyler.hpp"
#include "NodeSDLStylers/SVGBlockStyler.hpp"

#include "NodeEditorApp/SimulatorRunner.hpp"
#include "NodeEditorApp/BlockPropertiesDialog.hpp"
#include "NodeEditorApp/SimulationSettingsDialog.hpp"
#include "NodeEditorApp/NewSceneDialog.hpp"

#include "SceneLoader/SceneLoader.hpp"

static void AddInitialNodes_forScene(node::GraphicsObjectsManager* manager)
{
    assert(manager);
    using namespace node;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();

    {
        model::BlockModel model{ model::BlockId{3}, model::Rect{ 400,10,100,100 }};
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 0 } });
        model.SetClass("Ramp");
        model.SetStyler("SVG Styler");
        model.SetStylerProperties(model::BlockStyleProperties{ {{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/ramp.svg"}} });
        model.GetProperties().push_back(model::BlockProperty{ "Slope",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }

    {
        model::BlockModel model{ model::BlockId{4}, model::Rect{ 200,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetClass("Gain");
        model.SetStyler("Gain");
        model.GetProperties().push_back(model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 });
        sceneModel->AddBlock(std::move(model));
    }


    {
        model::BlockModel model{ model::BlockId{5}, model::Rect{ 400,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.SetClass("Scope Display");
        model.SetStyler("SVG Styler");
        model.SetStylerProperties(model::BlockStyleProperties{ {{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/scope.svg"}} });
        model.GetProperties().push_back(model::BlockProperty{ "Inputs",model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1) });
        sceneModel->AddBlock(std::move(model));
    }

    manager->SetSceneModel(std::make_shared<SceneModelManager>(std::move(sceneModel)));
}


void node::MainNodeScene::RunSimulator()
{
    assert(GetApp());
    assert(m_classesManager);
    m_sim_mgr.RunSimulator(m_graphicsObjectsManager->GetSceneModel()->GetModel(), m_classesManager, *GetApp());
}

void node::MainNodeScene::DeleteEventReceiver(const NodeSceneEventReceiver* handler)
{
    auto it = std::find_if(m_event_receivers.begin(), m_event_receivers.end(), [&](const auto& ptr) {return ptr.get() == handler; });
    assert(it != m_event_receivers.end());
    if (it != m_event_receivers.end())
    {
        m_event_receivers.erase(it);
    }
}

void node::MainNodeScene::NewScene()
{
    SDL_Log("new Scene confirmed!");
    // remember to save scene here

    m_db_connector = std::nullopt;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();
    CloseAllDialogs();
    m_graphicsObjectsManager->SetSceneModel(std::make_shared<SceneModelManager>(std::move(sceneModel)));
}

void node::MainNodeScene::LoadScene(std::string name)
{
    SDL_Log("load Scene: %s", name.c_str());
    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    if (auto new_scene = connector.connector->Load())
    {
        auto scene = std::make_shared<model::NodeSceneModel>(std::move(*new_scene));
        CloseAllDialogs();
        m_db_connector = std::move(connector);
        m_graphicsObjectsManager->SetSceneModel(std::make_shared<SceneModelManager>(std::move(scene)));
    }
}

void node::MainNodeScene::SaveScene()
{
    assert(m_db_connector);
    if (!m_db_connector)
    {
        return;
    }
    SDL_Log("scene Saved to %s", m_db_connector->db_path.c_str());
    m_db_connector->connector->Save(m_graphicsObjectsManager->GetSceneModel()->GetModel());
}

void node::MainNodeScene::SaveScene(std::string name)
{
    SDL_Log("scene Saved to %s", name.c_str());
    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    m_db_connector = std::move(connector);
    m_db_connector->connector->Save(m_graphicsObjectsManager->GetSceneModel()->GetModel());
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
    {
        auto new_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "New", [this]() {SDL_Log("New!"); this->NewScenePressed(); });
        new_btn->SetSVGPath("./assets/new_file.svg");
        new_btn->SetDescription("New");
        toolbar->AddButton(std::move(new_btn));
    }
    {
        auto load_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "Load", [this]() {SDL_Log("Load!"); this->LoadSceneButtonPressed(); });
        load_btn->SetSVGPath("./assets/load_file.svg");
        load_btn->SetDescription("Load");
        toolbar->AddButton(std::move(load_btn));
    }
    {
        auto save_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "Save", [this]() {SDL_Log("Save!"); this->SaveSceneButtonPressed(); });
        save_btn->SetDescription("Save");
        save_btn->SetSVGPath("./assets/save_file.svg");
        toolbar->AddButton(std::move(save_btn));
    }
    toolbar->AddSeparator();
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
    {
        auto arrow_btn = std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "A", m_toolsManager);
        arrow_btn->SetDescription("Arrow Tool");
        arrow_btn->SetSVGPath("./assets/arrow.svg");
        toolbar->AddButton(std::move(arrow_btn));
    }
    {
        auto net_tool = std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "N", m_toolsManager);
        net_tool->SetDescription("Net Tool");
        net_tool->SetSVGPath("./assets/net_tool.svg");
        toolbar->AddButton(std::move(net_tool));        
    }
    {
        auto delete_tool = std::make_unique<ToolButton>(SDL_Rect{ 0,0,40,40 }, this, "D", m_toolsManager);
        delete_tool->SetDescription("Delete Tool");
        delete_tool->SetSVGPath("./assets/delete_tool.svg");
        toolbar->AddButton(std::move(delete_tool));
    }
    toolbar->AddSeparator();
    {
        auto undo_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "U",
            [this]() { SDL_Log("Undo"); this->OnUndo(); }, [this] {return this->CanUndo(); });
        undo_btn->SetDescription("Undo");
        undo_btn->SetSVGPath("./assets/undo.svg");
        toolbar->AddButton(std::move(undo_btn));
    }
    {
        auto redo_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "R",
            [this]() { SDL_Log("Redo"); this->OnRedo(); }, [this] {return this->CanRedo(); });
        redo_btn->SetDescription("Redo");
        redo_btn->SetSVGPath("./assets/redo.svg");
        toolbar->AddButton(std::move(redo_btn));
    }

    toolbar->AddSeparator();
    {
        auto prop_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "P",
            [this]() {SDL_Log("Properties!"); this->OpenPropertiesDialog(); });
        prop_btn->SetDescription("Properties");
        prop_btn->SetSVGPath("./assets/properties.svg");
        toolbar->AddButton(std::move(prop_btn));
    }
    {
        auto run_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "R", [this]() {SDL_Log("Run!"); this->RunSimulator(); },
            [this]() { return !this->m_sim_mgr.IsSimulationRunning(); });
        run_btn->SetDescription("Run Simulation");
        run_btn->SetSVGPath("./assets/run.svg");
        toolbar->AddButton(std::move(run_btn));
    }
    {
        auto stop_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "S", [this]() {SDL_Log("Stop!"); this->m_sim_mgr.StopSimulator(); },
            [this]() { return this->m_sim_mgr.IsSimulationRunning(); });
        stop_btn->SetDescription("Stop Simulation");
        stop_btn->SetSVGPath("./assets/stop_sim.svg");
        toolbar->AddButton(std::move(stop_btn));
    }
    {
        auto settings_btn = std::make_unique<ToolBarCommandButton>(SDL_Rect{ 0,0,40,40 }, this, "T", [this]() {SDL_Log("Settings!"); this->OnSettingsClicked(); });
        settings_btn->SetDescription("Settings");
        settings_btn->SetSVGPath("./assets/settings.svg");
        toolbar->AddButton(std::move(settings_btn));
    }
    SetToolBar(std::move(toolbar));
    m_toolsManager->ChangeTool("A");
}

void node::MainNodeScene::InitializeSidePanel(node::GraphicsScene* gScene)
{
    assert(gScene);
    UNUSED_PARAM(gScene);
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_Rect{ 0,0,300,m_rect.h}, this);

    auto&& pallete_provider = std::make_shared<PalleteProvider>(m_classesManager, m_blockStylerFactory);
    auto block_template = BlockTemplate{
        "Gain",
        "Gain",
        "Gain",
        std::vector<model::BlockProperty>{
            model::BlockProperty{"Multiplier", model::BlockPropertyType::FloatNumber, 1.0}
        }
        ,
        model::BlockStyleProperties{}
    };


    for (int i = 0; i < 1; i++)
    {
        pallete_provider->AddElement(block_template);
    }

    auto add_block = BlockTemplate{
    "Add",
    "Add Simple",
    "Text",
    std::vector<model::BlockProperty>{},
    model::BlockStyleProperties{{{TextBlockStyler::key_text, "+"}}}
    };
    pallete_provider->AddElement(std::move(add_block));

    auto integrate_block = BlockTemplate{
        "Integration",
        "Integration",
        "SVG Styler",
        std::vector<model::BlockProperty>{},
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/integral.svg"}}}
    };
    pallete_provider->AddElement(std::move(integrate_block));

    auto deriv_block = BlockTemplate{
    "Derivative",
    "Derivative",
    "SVG Styler",
    std::vector<model::BlockProperty>{},
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/derivative.svg"}}}
    };
    pallete_provider->AddElement(std::move(deriv_block));

    auto constant_block = BlockTemplate{
        "Constant Source",
        "Constant Source",
        "Property Printer",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Value", model::BlockPropertyType::FloatNumber, 1.0}
        },
        model::BlockStyleProperties{{{PropertyPrintStyler::printed_key_text, "Value"}}}
    };
    pallete_provider->AddElement(std::move(constant_block));

    auto ramp_block = BlockTemplate{
        "Ramp",
        "Ramp",
        "SVG Styler",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Slope", model::BlockPropertyType::FloatNumber, 1.0}
        },
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/ramp.svg"}}}
    };
    pallete_provider->AddElement(std::move(ramp_block));

    auto scope_block = BlockTemplate{
        "Scope",
        "Scope Display",
        "SVG Styler",
        std::vector<model::BlockProperty>{
        model::BlockProperty{"Inputs", model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1)}
        },
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/scope.svg"}}}
    };
    pallete_provider->AddElement(std::move(scope_block));

    auto multiply_block = BlockTemplate{
    "Multiply",
    "Multiply",
    "Text",
    std::vector<model::BlockProperty>{
    },
    model::BlockStyleProperties{{{TextBlockStyler::key_text, "X"}}}
    };
    pallete_provider->AddElement(std::move(multiply_block));

    auto sine_block = BlockTemplate{
    "Sine",
    "Sine",
    "SVG Styler",
    std::vector<model::BlockProperty>{
        model::BlockProperty{"Phase_deg", model::BlockPropertyType::FloatNumber, 0.0},
        model::BlockProperty{"Freq_hz", model::BlockPropertyType::FloatNumber, 1.0},
    },
    model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/sine.svg"}}}
    };
    pallete_provider->AddElement(std::move(sine_block));

    auto step_block = BlockTemplate{
    "Step",
    "Step",
    "SVG Styler",
    std::vector<model::BlockProperty>{
        node::model::BlockProperty{"Initial Value", node::model::BlockPropertyType::FloatNumber, 0.0 },
        node::model::BlockProperty{"Final Value", node::model::BlockPropertyType::FloatNumber, 1.0 },
        node::model::BlockProperty{"Step Time", node::model::BlockPropertyType::FloatNumber, 1.0 },
    },
    model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "./assets/step.svg"}}}
    };
    pallete_provider->AddElement(std::move(step_block));

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

    OpenPropertiesDialog(*static_cast<BlockObject*>(object));
}

void node::MainNodeScene::OpenPropertiesDialog(BlockObject& object)
{
    {
        auto it = m_objects_dialogs.find(static_cast<BlockObject*>(&object));
        if (it != m_objects_dialogs.end() && it->second.dialog.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.dialog.GetObjectPtr());
            BumpDialogToTop(dialog);
            SDL_Rect dialog_rect = dialog->GetRect();
            dialog->SetRect({ 100,100,dialog_rect.w, dialog_rect.h });
            return;
        }
    }

    auto model_id = static_cast<BlockObject*>(&object)->GetModelId();
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
    auto dialog = std::make_unique<BlockPropertiesDialog>(*block, m_graphicsObjectsManager, m_classesManager, SDL_Rect{ 100,100,0,0 }, this);
    m_objects_dialogs[static_cast<BlockObject*>(&object)] = DialogSlot{ dialog->GetMIHandlePtr(), DialogType::PropertiesDialog };
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
        if (it != m_objects_dialogs.end() && it->second.dialog.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.dialog.GetObjectPtr());
            BumpDialogToTop(dialog);
            SDL_Rect dialog_rect = dialog->GetRect();
            dialog->SetRect({ 100,100,dialog_rect.w, dialog_rect.h });
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
            auto sim_results = m_sim_mgr.GetLastSimulationResults();
            auto block_it = std::find_if(sim_results.begin(), sim_results.end(),
                [&](const BlockResult& r) {return r.id == model_id; });
            if (block_it != sim_results.end())
            {
                sim_data = block_it->data;
            }
        }
        auto dialog = class_ptr->CreateBlockDialog(*this, *block_model, sim_data);
        if (dialog)
        {
            m_objects_dialogs[&block] = DialogSlot{ dialog->GetMIHandlePtr(), DialogType::BlockDialog };
            AddNormalDialog(std::move(dialog));
        }
    }
    else
    {
        OpenPropertiesDialog(block);
    }
}

void node::MainNodeScene::NewScenePressed()
{
    SetModalDialog(std::make_unique<NewSceneDialog>(SDL_Rect{ 100,100,0,0 }, this));
}

void node::MainNodeScene::LoadSceneButtonPressed()
{
    SetModalDialog(std::make_unique<LoadSceneDialog>(SDL_Rect{ 100,100,0,0 }, this));
}

void node::MainNodeScene::SaveSceneButtonPressed()
{
    if (m_db_connector)
    {
        SaveScene();
        return;
    }
    SetModalDialog(std::make_unique<SaveSceneDialog>(SDL_Rect{ 100,100,0,0 }, this));
}

void node::MainNodeScene::OnUndo()
{
    SDL_Log("Undoed!");
    if (!m_graphicsObjectsManager->GetSceneModel()->CanUndo())
    {
        assert(false);
        return;
    }
    m_graphicsObjectsManager->GetSceneModel()->Undo();
}

void node::MainNodeScene::OnRedo()
{
    SDL_Log("Redoed!");
    if (!m_graphicsObjectsManager->GetSceneModel()->CanRedo())
    {
        assert(false);
        return;
    }
    m_graphicsObjectsManager->GetSceneModel()->Redo();
}

bool node::MainNodeScene::CanUndo()
{
    return m_graphicsObjectsManager->GetSceneModel()->CanUndo();
}

bool node::MainNodeScene::CanRedo()
{
    return m_graphicsObjectsManager->GetSceneModel()->CanRedo();
}

node::MainNodeScene::MainNodeScene(SDL_Rect rect, node::Application* parent)
:Scene(rect, parent)
{
}

void node::MainNodeScene::OnInit()
{
    using namespace node;
    
    m_sim_mgr.SetSimulationEndCallback([this](const auto& evt) {this->OnSimulationEnd(evt); });

    m_blockStylerFactory = std::make_shared<BlockStylerFactory>();
    m_blockStylerFactory->AddStyler("Default", [](const model::BlockModel&) { return std::make_unique<DefaultBlockStyler>(); });
    m_blockStylerFactory->AddStyler("Text", [font = this->GetApp()->getFont().get()](const model::BlockModel& model) 
        { return TextBlockStyler::Create(model.GetStylerProperties(), font); });
    m_blockStylerFactory->AddStyler("Gain", [font = this->GetApp()->getFont().get()](const model::BlockModel& model)
        { return std::make_unique<GainBlockStyler>(model, font); });
    m_blockStylerFactory->AddStyler("Property Printer", [font = this->GetApp()->getFont().get()](const model::BlockModel& model)
        { return PropertyPrintStyler::Create(model, font); });
    m_blockStylerFactory->AddStyler("SVG Styler", [](const model::BlockModel& model)
        { return std::make_unique<SVGBlockStyler>(model); });

    std::unique_ptr<NodeGraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(m_rect, this);
    m_graphicsScene = static_cast<NodeGraphicsScene*>(gScene.get());
    m_graphicsObjectsManager = std::make_shared<GraphicsObjectsManager>(*gScene, m_blockStylerFactory);
    m_graphicsScene->Attach(*m_graphicsObjectsManager);

    m_classesManager = std::make_shared<BlockClassesManager>();
    m_classesManager->RegisterBlockClass(std::make_shared<GainBlockClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<ConstantSourceClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<ScopeDisplayClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<RampSourceClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<IntegrationBlockClass>());    
    m_classesManager->RegisterBlockClass(std::make_shared<DerivativeBlockClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<AddSimpleBlockClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<MultiplyBlockClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<SineSourceClass>());
    m_classesManager->RegisterBlockClass(std::make_shared<StepSourceClass>());

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

void node::MainNodeScene::OnSimulationEnd(const SimulationEvent& event)
{
    std::visit(overloaded{
        [](const SimulationEvent::NetFloatingError&)
        {
            SDL_Log("Floating Net!");
        },
        [](const SimulationEvent::Stopped&)
        {
            SDL_Log("Stopped!");
        },
        [](const SimulationEvent::OutputSocketsConflict&)
        {
            SDL_Log("Sockets Conflict!");
        },
        [](const SimulationEvent::FloatingInput&)
        {
           SDL_Log("Floating Input!");
        },
        [&](const SimulationEvent::Success&)
        {
            SDL_Log("Success!");
            for (auto&& block_result : m_sim_mgr.GetLastSimulationResults())
            {
                auto&& block_it = m_graphicsObjectsManager->getBlocksRegistry().find(block_result.id);
                if (block_it != m_graphicsObjectsManager->getBlocksRegistry().end())
                {
                    auto dialog_it = m_objects_dialogs.find(block_it->second);
                    if (dialog_it != m_objects_dialogs.end() && dialog_it->second.type == DialogType::BlockDialog && dialog_it->second.dialog.isAlive())
                    {
                        static_cast<BlockDialog*>(dialog_it->second.dialog.GetObjectPtr())->UpdateResults(block_result.data);
                    }
                }
            }
        }
        }, event.e);
    SDL_Log("simulation Ended!");
}

void node::MainNodeScene::OnSettingsClicked()
{
    if (!m_settings_dialog.isAlive())
    {
        auto dialog = std::make_unique<SimulationSettingsDialog>([this](const auto& result) {this->m_sim_mgr.SetSimulationSettings(result); },
            m_sim_mgr.GetSimulationSettings(), SDL_Rect{ 100,100,0,0 }, this);
        m_settings_dialog = dialog->GetMIHandlePtr();
        AddNormalDialog(std::move(dialog));
    }
    else
    {
        auto* dialog = static_cast<SimulationSettingsDialog*>(m_settings_dialog.GetObjectPtr());
        const auto& rect = dialog->GetRect();
        dialog->SetRect({ 100, 100, rect.w, rect.h });
        BumpDialogToTop(dialog);
    }
}

bool node::MainNodeScene::OnRMBUp(const SDL_Point& p)
{
    if (Scene::OnRMBUp(p))
    {
        return true;
    }
    //std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this);
    //this->ShowContextMenu(std::move(menu), {p.x, p.y});
    return true;
}

