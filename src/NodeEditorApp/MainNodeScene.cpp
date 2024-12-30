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
#include "GraphicsScene/tools/GraphicsToolHandler.hpp"

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
#include "BlockClasses/ComparatorClass.hpp"

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
#include "NodeEditorApp/SceneManager.hpp"

#include "SceneLoader/SceneLoader.hpp"
#include <filesystem>
#include <limits>
#include "toolgui/TabbedView.hpp"

#define USE_SDL_DIALOGS 1

static void AddInitialNodes_forScene(node::SceneManager& manager)
{
    using namespace node;
    auto sceneModel = std::make_shared<model::NodeSceneModel>();

    {
        auto block_id = model::BlockId{ 3 };
        model::BlockModel model{ block_id, model::BlockType::Functional, model::Rect{ 400,10,100,100 }};
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 0 } });
        model.SetStyler("SVG Styler");
        model.SetStylerProperties(model::BlockStyleProperties{ {{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/ramp.svg"}} });
        sceneModel->GetFunctionalBlocksManager().SetDataForId(block_id, 
            model::FunctionalBlockData{ "Ramp" ,
            {
                model::BlockProperty{ "Slope",model::BlockPropertyType::FloatNumber, 1.0 }
            }
            });
        sceneModel->AddBlock(std::move(model));
    }

    {
        auto block_id = model::BlockId{ 4 };
        model::BlockModel model{ block_id, model::BlockType::Functional, model::Rect{ 200,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetStyler("Gain");
        sceneModel->GetFunctionalBlocksManager().SetDataForId(block_id,
            model::FunctionalBlockData{ "Gain" ,
            {
                model::BlockProperty{ "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 }
            }
            });
        sceneModel->AddBlock(std::move(model));
    }


    {
        auto block_id = model::BlockId{ 5 };
        model::BlockModel model{ block_id, model::BlockType::Functional, model::Rect{ 400,210,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.SetStyler("SVG Styler");
        model.SetStylerProperties(model::BlockStyleProperties{ {{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/scope.svg"}} });
        sceneModel->GetFunctionalBlocksManager().SetDataForId(block_id,
            model::FunctionalBlockData{ "Scope Display" ,
            {
                model::BlockProperty{ "Inputs",model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1) }
            }
            });
        sceneModel->AddBlock(std::move(model));
    }

    auto model = std::make_shared<SceneModelManager>(std::move(sceneModel));
    model->SetSubSceneId(manager.GetMainSubSceneId());
    manager.AddModel(model);
}


void node::MainNodeScene::RunSimulator()
{
    if (!m_current_scene_id)
    {
        return;
    }
    assert(GetApp());
    assert(m_classesManager);
    std::vector<std::reference_wrapper<const model::NodeSceneModel>> scene_models;
    auto& scene_manager = m_sceneComponents[m_current_scene_id->manager].manager;
    for (auto&& subscene : scene_manager->GetModels())
    {
        scene_models.push_back(subscene.second->GetModel());
    }
    m_sim_mgr.RunSimulator(m_current_scene_id->manager, scene_models, scene_manager->GetMainSubSceneId(), 
        m_classesManager, *GetApp());
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

    if (m_sim_mgr.IsSimulationRunning())
    {
        m_sim_mgr.StopSimulator();
    }
    m_sim_mgr.ClearLastSimulationReults();

    CloseAllDialogs();

    auto model = std::make_shared<SceneModelManager>(std::make_shared<model::NodeSceneModel>());
    
    auto manager_id = CreateNewScene();
    assert(m_sceneComponents.find(manager_id) != m_sceneComponents.end());
    auto& component = m_sceneComponents[manager_id];
    auto subscene_id = component.manager->GetMainSubSceneId();
    m_current_scene_id = SceneId{ manager_id, subscene_id };
    
    model->SetSubSceneId(subscene_id);
    
    component.manager->AddModel(std::move(model));
    
    CreateSceneForSubsystem(*m_current_scene_id);

    auto mgr = component.manager->GetManager(subscene_id);
    m_tabbedView->SetCurrentTabIndex(mgr->GetGraphicsScene());
}


static std::optional<std::unordered_map<node::SubSceneId, std::shared_ptr<node::model::NodeSceneModel>>>
LoadSceneAndSubscenes(node::loader::SceneLoader& db_conn, node::SubSceneId id)
{
    using namespace node;
    std::unordered_map<SubSceneId, std::shared_ptr<model::NodeSceneModel>> scenes;
    SDL_Log("Loading Scene %d", id.value);
    auto scene = db_conn.Load(id);
    if (!scene)
    {
        SDL_Log("Failed to Load Scene %d", id.value);
        return std::nullopt;
    }

    auto scene_ptr = std::make_shared<model::NodeSceneModel>(std::move(*scene));
    scenes[id] = std::move(scene_ptr);

    auto child_scene_ids = db_conn.GetChildSubScenes(id);
    if (!child_scene_ids)
    {
        return std::nullopt;
    }

    for (const auto& child_id : *child_scene_ids)
    {
        auto child_scenes = LoadSceneAndSubscenes(db_conn, child_id);
        if (!child_scenes)
        {
            return std::nullopt;
        }
        for (auto&& [grand_child_id, child_scene]: *child_scenes)
        {
            scenes[grand_child_id] = std::move(child_scene);
        }
    }
    return scenes;
}

void node::MainNodeScene::LoadScene(std::string name)
{
    SDL_Log("load Scene: %s", name.c_str());

    if (m_sim_mgr.IsSimulationRunning())
    {
        m_sim_mgr.StopSimulator();
    }
    m_sim_mgr.ClearLastSimulationReults();

    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    if (auto new_scenes = LoadSceneAndSubscenes(*connector.connector,SubSceneId{1}))
    {
        CloseAllDialogs();
        auto manager_id = CreateNewScene();
        assert(m_sceneComponents.find(manager_id) != m_sceneComponents.end());
        auto& component = m_sceneComponents[manager_id];
        auto subscene_id = component.manager->GetMainSubSceneId();
        m_current_scene_id = SceneId{ manager_id, subscene_id };
        component.manager->SetDBConnector(std::move(connector));
        auto main_scene_it = new_scenes->find(subscene_id);
        assert(main_scene_it != new_scenes->end());
        if (main_scene_it == new_scenes->end())
        {
            return;
        }
        auto model = std::make_shared<SceneModelManager>(std::move(main_scene_it->second));
        assert(model->GetSubSceneId() == component.manager->GetMainSubSceneId());
        new_scenes->erase(main_scene_it);
        component.manager->AddModel(std::move(model));
        for (auto&& [id, child_scene] : *new_scenes)
        {
            if (child_scene)
            {
                component.manager->AddModel(std::make_shared<SceneModelManager>(std::move(child_scene)));
            }
        }
        SDL_Log("Loading Scene successful!");
        CreateSceneForSubsystem({ manager_id, component.manager->GetMainSubSceneId() });
        auto mgr = component.manager->GetManager(component.manager->GetMainSubSceneId());
        m_tabbedView->SetCurrentTabIndex(mgr->GetGraphicsScene());
    }
    else
    {
        std::string error_message;
        if (std::filesystem::is_regular_file(connector.db_path))
        {
            error_message = "Could not load file: \n" + connector.db_path;            
        }
        else
        {
            error_message = "File doesn't exist: \n" + connector.db_path;
        }
#ifndef USE_SDL_DIALOGS
        auto dialog = std::make_unique<OkCancelModalDialog>("Load Failed!", std::vector<std::string>{ std::move(error_message) },
            SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this, true);
        SetModalDialog(std::move(dialog));
#else
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Load Failed!", error_message.c_str(), GetApp()->GetWindow());
#endif
    }
}

static bool SaveSubSceneAndChildren(node::loader::SceneLoader& db_conn, node::SceneManager& components, node::SubSceneId id, node::SubSceneId parent_id)
{
    using namespace node;
    SDL_Log("Saving Scene with id %d", id.value);
    auto ModelManager = components.GetModel(id);
    if (!ModelManager)
    {
        SDL_Log("Scene Model not found! %d", id.value);
        assert(ModelManager);
        return false;
    }

    if (!db_conn.Save(ModelManager->GetModel(), id, parent_id))
    {
        return false;
    }
    for (auto&& [block_id, subsystemData] : ModelManager->GetModel().GetSubsystemBlocksManager().GetData())
    {
        if (subsystemData.URL == "Local")
        {
            if (!SaveSubSceneAndChildren(db_conn, components, subsystemData.scene_id, id))
            {
                return false;
            }
        }
    }
    return true;
}

void node::MainNodeScene::SaveScene()
{
    if (!m_current_scene_id)
    {
        return;
    }

    auto& mgr = m_sceneComponents[m_current_scene_id->manager].manager;
    auto& db_connector = mgr->GetDBConnector();
    auto graphicsObjectsManager = mgr->GetManager(m_current_scene_id->subscene);
    assert(db_connector);
    if (!db_connector)
    {
        return;
    }
    SDL_Log("scene Saved to %s", db_connector->db_path.c_str());
    if (db_connector->connector->Reset())
    {
        SaveSubSceneAndChildren(*db_connector->connector, *m_sceneComponents[m_current_scene_id->manager].manager, mgr->GetMainSubSceneId(), SubSceneId{ 0 });
    }
    SDL_Log("saving Scene Success!");
}

void node::MainNodeScene::SaveScene(std::string name)
{
    if (!m_current_scene_id)
    {
        return;
    }

    auto& mgr = m_sceneComponents[m_current_scene_id->manager].manager;
    auto& db_connector = mgr->GetDBConnector();
    auto graphicsObjectsManager = mgr->GetManager(m_current_scene_id->subscene);
    SDL_Log("scene Saved to %s", name.c_str());
    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    if (connector.connector->Reset() && 
        SaveSubSceneAndChildren(*connector.connector, 
            *m_sceneComponents[m_current_scene_id->manager].manager, mgr->GetMainSubSceneId(), SubSceneId{ 0 }))
    {
        db_connector = std::move(connector);
        SDL_Log("saving Scene Success!");
    }
    else
    {
#ifndef USE_SDL_DIALOGS
        auto dialog = std::make_unique<OkCancelModalDialog>("Save Failed!", std::vector<std::string>{ "Failed to write to File: "+ connector.db_path,
        "File may be innaccessible or is an invalid format or read only!"},
            SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this, true);
        SetModalDialog(std::move(dialog));
#else
        std::string str = "Failed to write to File: \n" + connector.db_path;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Save Failed!", str.c_str(), GetApp()->GetWindow());
#endif
    }
}

void node::MainNodeScene::MaybeSaveScene(std::string name)
{
    SDL_Log("scene Maybe Saved to %s", name.c_str());
    if (std::filesystem::is_regular_file(name))
    {
        auto dialog = std::make_unique<ConfirmOverwriteSaveSceneDialog>(std::move(name), SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this);
        SetModalDialog(std::move(dialog));
    }
    else
    {
        SaveScene(std::move(name));
    }
}

void node::MainNodeScene::OnCenterTabChanged(int32_t new_tab)
{
    m_current_scene_id = GetSceneIdForTab(new_tab);
}

void node::MainNodeScene::CloseTabRequest(int32_t tab_idx)
{
    auto sceneId_opt = GetSceneIdForTab(tab_idx);
    assert(sceneId_opt);
    if (!sceneId_opt)
    {
        SDL_Log("closing unavailable tab %d", static_cast<int>(tab_idx));
        return;
    }

    auto sceneId = *sceneId_opt;
    m_tabbedView->DeleteTab(tab_idx);
    auto& component = m_sceneComponents[sceneId.manager];
    if (sceneId.subscene == component.manager->GetMainSubSceneId())
    {
        for (auto& manager: component.manager->GetManagers())
        {
            if (manager.second->GetGraphicsScene())
            {
                if (auto idx = m_tabbedView->GetWidgetIndex(manager.second->GetGraphicsScene()))
                {
                    m_tabbedView->DeleteTab(*idx);
                }
            }
        }
        m_sceneComponents.erase(sceneId.manager);
    }
    else
    {
        m_sceneComponents[sceneId.manager].manager->SetSubSceneManager(sceneId.subscene, nullptr);
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
    auto toolbar = std::make_unique<ToolBar>(SDL_FRect{ 0.0f,0.0f,0.0f,0.0f }, this);
    {
        auto new_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "New", [this]() {SDL_Log("New!"); this->NewScenePressed(); });
        new_btn->SetSVGPath("assets/new_file.svg");
        new_btn->SetDescription("New");
        toolbar->AddButton(std::move(new_btn));
    }
    {
        auto load_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "Load", [this]() {SDL_Log("Load!"); this->LoadSceneButtonPressed(); });
        load_btn->SetSVGPath("assets/load_file.svg");
        load_btn->SetDescription("Load");
        toolbar->AddButton(std::move(load_btn));
    }
    {
        auto save_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "Save", [this]() {SDL_Log("Save!"); this->SaveSceneButtonPressed(); });
        save_btn->SetDescription("Save");
        save_btn->SetSVGPath("assets/save_file.svg");
        toolbar->AddButton(std::move(save_btn));
    }
    toolbar->AddSeparator();
    m_toolsManager = std::make_shared<ToolsManager>(toolbar.get());
    {
        auto arrow_tool = std::make_shared<ArrowTool>();
        auto receiver = std::make_unique<DoubleClickEventReceiver>(*this, [this](BlockObject& block) {this->OpenBlockDialog(block); } );
        arrow_tool->Attach(*receiver);
        AddEventReceiver(std::move(receiver));
        m_toolsManager->AddTool("A", std::move(arrow_tool));
    }
    m_toolsManager->AddTool("D", std::make_shared<DeleteTool>());
    m_toolsManager->AddTool("N", std::make_shared<NetTool>());
    {
        auto arrow_btn = std::make_unique<ToolButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "A", m_toolsManager);
        arrow_btn->SetDescription("Arrow Tool");
        arrow_btn->SetSVGPath("assets/arrow.svg");
        toolbar->AddButton(std::move(arrow_btn));
    }
    {
        auto net_tool = std::make_unique<ToolButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "N", m_toolsManager);
        net_tool->SetDescription("Net Tool");
        net_tool->SetSVGPath("assets/net_tool.svg");
        toolbar->AddButton(std::move(net_tool));        
    }
    {
        auto delete_tool = std::make_unique<ToolButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "D", m_toolsManager);
        delete_tool->SetDescription("Delete Tool");
        delete_tool->SetSVGPath("assets/delete_tool.svg");
        toolbar->AddButton(std::move(delete_tool));
    }
    toolbar->AddSeparator();
    {
        auto undo_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "U",
            [this]() { SDL_Log("Undo"); this->OnUndo(); }, [this] {return this->CanUndo(); });
        undo_btn->SetDescription("Undo");
        undo_btn->SetSVGPath("assets/undo.svg");
        toolbar->AddButton(std::move(undo_btn));
    }
    {
        auto redo_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "R",
            [this]() { SDL_Log("Redo"); this->OnRedo(); }, [this] {return this->CanRedo(); });
        redo_btn->SetDescription("Redo");
        redo_btn->SetSVGPath("assets/redo.svg");
        toolbar->AddButton(std::move(redo_btn));
    }

    toolbar->AddSeparator();
    {
        auto prop_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "P",
            [this]() {SDL_Log("Properties!"); this->OpenPropertiesDialog(); });
        prop_btn->SetDescription("Properties");
        prop_btn->SetSVGPath("assets/properties.svg");
        toolbar->AddButton(std::move(prop_btn));
    }
    {
        auto run_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "R", [this]() {SDL_Log("Run!"); this->RunSimulator(); },
            [this]() { return !this->m_sim_mgr.IsSimulationRunning(); });
        run_btn->SetDescription("Run Simulation");
        run_btn->SetSVGPath("assets/run.svg");
        toolbar->AddButton(std::move(run_btn));
    }
    {
        auto stop_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "S", [this]() {SDL_Log("Stop!"); this->m_sim_mgr.StopSimulator(); },
            [this]() { return this->m_sim_mgr.IsSimulationRunning(); });
        stop_btn->SetDescription("Stop Simulation");
        stop_btn->SetSVGPath("assets/stop_sim.svg");
        toolbar->AddButton(std::move(stop_btn));
    }
    {
        auto settings_btn = std::make_unique<ToolBarCommandButton>(SDL_FRect{ 0.0f,0.0f,40.0f,40.0f }, toolbar.get(), "T", [this]() {SDL_Log("Settings!"); this->OnSettingsClicked(); });
        settings_btn->SetDescription("Settings");
        settings_btn->SetSVGPath("assets/settings.svg");
        toolbar->AddButton(std::move(settings_btn));
    }
    SetToolBar(std::move(toolbar));
    m_toolsManager->ChangeTool("A");
}

void node::MainNodeScene::InitializeSidePanel()
{
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, SDL_FRect{ 0.0f,0.0f,300.0f,GetRect().h}, this);


    auto&& pallete_provider = std::make_shared<PalleteProvider>(m_classesManager, m_blockStylerFactory);

    auto SubScene_block = BlockTemplate
    {
        "SubSystem",
        model::SubsystemBlockData{
            "Local",
            SubSceneId{0}
        },
        "Default",
        model::BlockStyleProperties{}
    };
    pallete_provider->AddElement(std::move(SubScene_block));

    auto input_block = BlockTemplate
    {
        "Input",
        model::PortBlockData{
            model::SocketId{0},
            model::SocketType::input
        },
        "Default",
        model::BlockStyleProperties{}
    };
    pallete_provider->AddElement(std::move(input_block));

    auto output_block = BlockTemplate
    {
        "Output",
        model::PortBlockData{
            model::SocketId{0},
            model::SocketType::output
        },
        "Default",
        model::BlockStyleProperties{}
    };
    pallete_provider->AddElement(std::move(output_block));

    auto block_template = BlockTemplate{
    "Gain",
    model::FunctionalBlockData{
        "Gain",
        std::vector<model::BlockProperty>{
            model::BlockProperty{"Multiplier", model::BlockPropertyType::FloatNumber, 1.0}
        }
    },
    "Gain",
    model::BlockStyleProperties{}
    };
    for (int i = 0; i < 1; i++)
    {
        pallete_provider->AddElement(block_template);
    }

    auto add_block = BlockTemplate{
        "Add",
        model::FunctionalBlockData{
            "Add Simple", {}
        },
        "Text",
        model::BlockStyleProperties{{{TextBlockStyler::key_text, "+"}}}
    };
    pallete_provider->AddElement(std::move(add_block));

    auto integrate_block = BlockTemplate{
        "Integration",
        model::FunctionalBlockData{
            "Integration", {}
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/integral.svg"}}}
    };
    pallete_provider->AddElement(std::move(integrate_block));

    auto deriv_block = BlockTemplate{
        "Derivative",
        model::FunctionalBlockData{
            "Derivative", {}
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/derivative.svg"}}}
    };
    pallete_provider->AddElement(std::move(deriv_block));

    auto constant_block = BlockTemplate{
        "Constant Source",
        model::FunctionalBlockData{
            "Constant Source",
            std::vector<model::BlockProperty>{
                model::BlockProperty{"Value", model::BlockPropertyType::FloatNumber, 1.0}
            }
        },
        "Property Printer",
        model::BlockStyleProperties{{{PropertyPrintStyler::printed_key_text, "Value"}}}
    };
    pallete_provider->AddElement(std::move(constant_block));

    auto ramp_block = BlockTemplate{
        "Ramp",
        model::FunctionalBlockData{
            "Ramp",
            std::vector<model::BlockProperty>{
                model::BlockProperty{"Slope", model::BlockPropertyType::FloatNumber, 1.0}
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/ramp.svg"}}}
    };
    pallete_provider->AddElement(std::move(ramp_block));

    auto scope_block = BlockTemplate{
        "Scope",
        model::FunctionalBlockData {
            "Scope Display",
            std::vector<model::BlockProperty>{
                model::BlockProperty{"Inputs", model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1)}
            },
        },
        "SVG Styler",        
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/scope.svg"}}}
    };
    pallete_provider->AddElement(std::move(scope_block));

    auto multiply_block = BlockTemplate{
        "Multiply",
        model::FunctionalBlockData{
            "Multiply", {}
        },
        "Text",
        model::BlockStyleProperties{{{TextBlockStyler::key_text, "X"}}}
    };
    pallete_provider->AddElement(std::move(multiply_block));

    auto sine_block = BlockTemplate{
        "Sine",
        model::FunctionalBlockData{
            "Sine",
            std::vector<model::BlockProperty>{
                model::BlockProperty{"Phase_deg", model::BlockPropertyType::FloatNumber, 0.0},
                model::BlockProperty{"Freq_hz", model::BlockPropertyType::FloatNumber, 1.0},
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/sine.svg"}}}
    };
    pallete_provider->AddElement(std::move(sine_block));

    auto step_block = BlockTemplate{
        "Step",
        model::FunctionalBlockData{
            "Step",
            std::vector<model::BlockProperty>{
                node::model::BlockProperty{"Initial Value", node::model::BlockPropertyType::FloatNumber, 0.0 },
                node::model::BlockProperty{"Final Value", node::model::BlockPropertyType::FloatNumber, 1.0 },
                node::model::BlockProperty{"Step Time", node::model::BlockPropertyType::FloatNumber, 1.0 },
                node::model::BlockProperty{"Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 },
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/step.svg"}}}
    };
    pallete_provider->AddElement(std::move(step_block));

    auto comparator_block = BlockTemplate{
    "Comparator",
    model::FunctionalBlockData{
        "Comparator",
        std::vector<model::BlockProperty>{
            node::model::BlockProperty{"threshold", node::model::BlockPropertyType::FloatNumber, 0.0 },
            node::model::BlockProperty{"Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 },
        }
    },
    "SVG Styler",
    model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/comparator.svg"}}}
    };
    pallete_provider->AddElement(std::move(comparator_block));

    sidePanel->SetWidget(std::make_unique<BlockPallete>(SDL_FRect{ 0.0f,0.0f,200.0f,200.0f },
        std::move(pallete_provider), sidePanel.get()));
    SetSidePanel(std::move(sidePanel));
}

void node::MainNodeScene::OpenPropertiesDialog()
{
    if (!m_current_scene_id)
    {
        return;
    }
    auto* centerWidget = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene)->GetGraphicsScene();
    auto&& selection = centerWidget->GetCurrentSelection();
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
    if (!m_current_scene_id)
    {
        return;
    }

    auto& objects_dialogs = m_sceneComponents[m_current_scene_id->manager].manager->GetDialogs(m_current_scene_id->subscene);
    {
        auto it = objects_dialogs.find(static_cast<BlockObject*>(&object));
        if (it != objects_dialogs.end() && it->second.dialog.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.dialog.GetObjectPtr());
            BumpDialogToTop(dialog);
            SDL_FRect dialog_rect = dialog->GetRect();
            dialog->SetRect({ 100.0f,100.0f,dialog_rect.w, dialog_rect.h });
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
    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    auto* block = graphicsObjectsManager->GetSceneModel()->GetModel().GetBlockById(*model_id);
    if (!block)
    {
        SDL_Log("couldn't find the block model!");
        return;
    }

    assert(m_classesManager);
    auto dialog = std::make_unique<BlockPropertiesDialog>(*block, graphicsObjectsManager, m_classesManager, SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this);
    objects_dialogs[static_cast<BlockObject*>(&object)] = DialogSlot{ dialog->GetMIHandlePtr(), DialogType::PropertiesDialog };
    auto dialog_ptr = dialog.get();
    AddNormalDialog(std::move(dialog));
    SetFocus(dialog_ptr);
}

void node::MainNodeScene::OpenBlockDialog(node::BlockObject& block)
{
    if (!m_current_scene_id)
    {
        return;
    }

    auto manager_it = m_sceneComponents.find(m_current_scene_id->manager);
    if (manager_it == m_sceneComponents.end())
    {
        assert(false);
        return;
    }
    if (!manager_it->second.manager)
    {
        assert(false);
        return;
    }

    auto& manager = *manager_it->second.manager;
    auto& objects_dialogs = manager.GetDialogs(m_current_scene_id->subscene);

    if (!block.GetModelId())
    {
        assert(false);
        return;
    }

    {
        auto it = objects_dialogs.find(&block);
        if (it != objects_dialogs.end() && it->second.dialog.isAlive())
        {
            Dialog* dialog = static_cast<Dialog*>(it->second.dialog.GetObjectPtr());
            BumpDialogToTop(dialog);
            SDL_FRect dialog_rect = dialog->GetRect();
            dialog->SetRect({ 100.0f,100.0f,dialog_rect.w, dialog_rect.h });
            return;
        }
    }

    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);

    assert(block.GetModelId());
    auto* block_model_ptr = graphicsObjectsManager->GetSceneModel()->GetModel().GetBlockById(*block.GetModelId());
    assert(block_model_ptr);
    if (!block_model_ptr)
    {
        return;
    }
    auto& block_model = *block_model_ptr;
    if (block_model.GetType() == model::BlockType::Functional)
    {

        auto* block_data = manager.GetModel(m_current_scene_id->subscene)->GetModel().GetFunctionalBlocksManager().GetDataForId(block_model.GetId());
        assert(block_data);
        if (!block_data)
        {
            return;
        }

        auto class_ptr = m_classesManager->GetBlockClassByName(block_data->block_class);
        if (!class_ptr)
        {
            SDL_Log("class '%s' not found!", block_data->block_class.c_str());
            return;
        }
        if (class_ptr->HasBlockDialog())
        {
            auto sim_data = std::any{};
            {
                auto model_id = *block.GetModelId();
                const auto& sim_results = m_sceneComponents[m_current_scene_id->manager].manager->GetLastSimulationResults();
                auto block_it = std::find_if(sim_results.begin(), sim_results.end(),
                    [&](const BlockResult& r) {return r.id == model_id; });
                if (block_it != sim_results.end())
                {
                    sim_data = block_it->data;
                }
            }
            auto dialog = class_ptr->CreateBlockDialog(*this, block_model, *block_data,sim_data);
            if (dialog)
            {
                objects_dialogs[&block] = DialogSlot{ dialog->GetMIHandlePtr(), DialogType::BlockDialog };
                auto dialog_ptr = dialog.get();
                AddNormalDialog(std::move(dialog));
                SetFocus(dialog_ptr);
            }
        }
        else
        {
            OpenPropertiesDialog(block);
        }
    }
    else if (block_model.GetType() == model::BlockType::SubSystem)
    {
        auto* subsystem_block_data = manager.GetModel(m_current_scene_id->subscene)->GetModel().GetSubsystemBlocksManager().GetDataForId(block_model.GetId());
        assert(subsystem_block_data);
        if (!subsystem_block_data)
        {
            return;
        }

        if (subsystem_block_data->URL != "Local")
        {
            SDL_Log("Can only handle local blocks");
            return;
        }

        auto subsystem_id = subsystem_block_data->scene_id;
        
        
        // try getting the subscene if it exists
        auto new_subscene_mgr = manager.GetManager(subsystem_id);
        if (!new_subscene_mgr)
        {
            // scene is not displayed
            if (!CreateSceneForSubsystem({ m_current_scene_id->manager, subsystem_id }))
            {
                SDL_Log("Failed to create scene for subsystem %d,%d", 
                    m_current_scene_id->manager.value, subsystem_id.value);
                assert(false);
                return;
            }
            new_subscene_mgr = manager.GetManager(subsystem_id);
        }
        m_tabbedView->SetCurrentTabIndex(new_subscene_mgr->GetGraphicsScene());
    }
    else
    {
        SDL_Log("Openning Dialog Not handled!");
    }
    
}

void node::MainNodeScene::NewScenePressed()
{
    NewScene();
}

void node::MainNodeScene::LoadSceneButtonPressed()
{
#ifndef USE_SDL_DIALOGS
    SetModalDialog(std::make_unique<LoadSceneDialog>(SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this));
#else
    SDL_DialogFileCallback callback = +[](void* data, const char* const* file_list, int filter)
        {
            UNUSED_PARAM(filter);

            if (!file_list || !file_list[0])
            {
                return;
            }
            
            auto* scene = reinterpret_cast<node::MainNodeScene*>(data);
            assert(scene);
            if (!scene)
            {
                return;
            }
            scene->LoadScene(file_list[0]);
        };
    static constexpr SDL_DialogFileFilter filters[]{ {"blocks file (*.blks)", "blks"}, {"Any File (*.*)","*"} };
    SDL_ShowOpenFileDialog(callback, this, GetApp()->GetWindow(), filters, static_cast<int>(std::size(filters)), "scene.blks", false);
#endif
}

void node::MainNodeScene::SaveSceneButtonPressed()
{
    if (!m_current_scene_id)
    {
        return;
    }

    auto& db_connector = m_sceneComponents[m_current_scene_id->manager].manager->GetDBConnector();
    if (db_connector)
    {
        SaveScene();
        return;
    }
#ifndef USE_SDL_DIALOGS
    SetModalDialog(std::make_unique<SaveSceneDialog>(SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this));
#else
    SDL_DialogFileCallback callback = +[](void* data, const char* const* file_list, int filter)
        {
            UNUSED_PARAM(filter);

            if (!file_list || !file_list[0])
            {
                return;
            }

            auto* scene = reinterpret_cast<node::MainNodeScene*>(data);
            assert(scene);
            if (!scene)
            {
                return;
            }
            scene->MaybeSaveScene(file_list[0]);
        };
    static constexpr SDL_DialogFileFilter filters[]{ {"blocks file (*.blks)", "blks"}, {"Any File (*.*)","*"} };
    SDL_ShowSaveFileDialog(callback, this, GetApp()->GetWindow(), filters, static_cast<int>(std::size(filters)), "scene.blks");
#endif
}

node::SceneManagerId node::MainNodeScene::CreateNewScene()
{
    auto scene_mgr = std::make_unique<SceneManager>();
    scene_mgr->SetBlockStylerFactory(m_blockStylerFactory);
    auto new_id = SceneManagerId{ m_next_manager_id };
    m_sceneComponents.emplace(
        new_id,
        SceneComponents{
            std::move(scene_mgr),
        });
    m_next_manager_id++;
    return new_id;
}

bool node::MainNodeScene::CreateSceneForSubsystem(SceneId scene_id)
{

    auto scene_mgr_it = m_sceneComponents.find(scene_id.manager);
    if (scene_mgr_it == m_sceneComponents.end())
    {
        return false;
    }
    
    auto& scene_mgr = *scene_mgr_it->second.manager;
    std::unique_ptr<NodeGraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(GetRect(), m_tabbedView);
    auto graphicsObjectsManager = std::make_shared<GraphicsObjectsManager>(*gScene, m_blockStylerFactory);
    gScene->Attach(*graphicsObjectsManager);
    gScene->SetToolsManager(m_toolsManager);
    gScene->SetObjectsManager(graphicsObjectsManager);
    auto handler = std::make_shared<GraphicsToolHandler>(*gScene, graphicsObjectsManager, m_toolsManager);
    gScene->SetTool(std::move(handler));

    scene_mgr.SetSubSceneManager(scene_id.subscene, graphicsObjectsManager);
    

    m_tabbedView->AddTab("scene", std::move(gScene));
    return true;
}

void node::MainNodeScene::OnUndo()
{
    SDL_Log("Undoed!");
    if (!m_current_scene_id)
    {
        return;
    }

    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    if (!graphicsObjectsManager->GetSceneModel()->CanUndo())
    {
        assert(false);
        return;
    }
    graphicsObjectsManager->GetSceneModel()->Undo();
}

void node::MainNodeScene::OnRedo()
{
    SDL_Log("Redoed!");
    if (!m_current_scene_id)
    {
        return;
    }

    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    if (!graphicsObjectsManager->GetSceneModel()->CanRedo())
    {
        assert(false);
        return;
    }
    graphicsObjectsManager->GetSceneModel()->Redo();
}

bool node::MainNodeScene::CanUndo()
{
    if (!m_current_scene_id)
    {
        return false;
    }

    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    assert(graphicsObjectsManager);
    return graphicsObjectsManager->GetSceneModel()->CanUndo();
}

bool node::MainNodeScene::CanRedo()
{
    if (!m_current_scene_id)
    {
        return false;
    }
    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    assert(graphicsObjectsManager);
    return graphicsObjectsManager->GetSceneModel()->CanRedo();
}

std::optional<node::SceneId> node::MainNodeScene::GetSceneIdForTab(int32_t index)
{
    if (index == -1)
    {
        return std::nullopt;
    }
    auto tab_widget = m_tabbedView->GetTabWidget(index);
    for (auto&& component : m_sceneComponents)
    {
        auto& managers = component.second.manager->GetManagers();
        auto it = std::find_if(managers.begin(),
            managers.end(),
            [&](const auto& manager)
            {
                return manager.second->GetGraphicsScene() == tab_widget;
            });
        if (it != managers.end())
        {
            return SceneId{ component.first, it->first };
            break;
        }
    }
    return std::nullopt;
}

node::MainNodeScene::MainNodeScene(SDL_FRect rect, node::Application* parent)
:Scene(rect, parent)
{
}

namespace node
{

struct TabChangeNotifier : public SingleObserver<TabsChangeEvent>
{
    TabChangeNotifier(MainNodeScene& scene) : m_scene{ &scene } {}
    void OnNotify(TabsChangeEvent& e) override 
    { 
        if (std::holds_alternative<TabIndexChangeEvent>(e.e))
        {
            auto&& e2 = std::get<TabIndexChangeEvent>(e.e);
            m_scene->OnCenterTabChanged(e2.new_tab_idx);
        }
        if (std::holds_alternative<TabCloseRequestEvent>(e.e))
        {
            auto&& e2 = std::get<TabCloseRequestEvent>(e.e);
            m_scene->CloseTabRequest(e2.tab_idx);
        }
    }
    
    MainNodeScene* m_scene;
};
}

void node::MainNodeScene::OnInit()
{
    using namespace node;
    
    m_sim_mgr.SetSimulationEndCallback([this](const auto& evt) {this->OnSimulationEnd(evt); });

    m_blockStylerFactory = std::make_shared<BlockStylerFactory>();
    m_blockStylerFactory->AddStyler("Default", [](const model::BlockDataCRef&) { return std::make_unique<DefaultBlockStyler>(); });
    m_blockStylerFactory->AddStyler("Text", [font = this->GetApp()->getFont().get()](const model::BlockDataCRef& model)
        { return TextBlockStyler::Create(model.block.get().GetStylerProperties(), font); });
    m_blockStylerFactory->AddStyler("Gain", [font = this->GetApp()->getFont().get()](const model::BlockDataCRef& model)
        { return std::make_unique<GainBlockStyler>(model, font); });
    m_blockStylerFactory->AddStyler("Property Printer", [font = this->GetApp()->getFont().get()](const model::BlockDataCRef& model)
        { return std::make_unique<PropertyPrintStyler>(model, font); });
    m_blockStylerFactory->AddStyler("SVG Styler", [](const model::BlockDataCRef& model)
        { return std::make_unique<SVGBlockStyler>(model.block); });

    {
        std::unique_ptr<TabbedView> view = std::make_unique<TabbedView>(GetApp()->getFont().get(), SDL_FRect{ 0,0,0,0 }, this);
        m_tabbedView = view.get();
        SetCenterWidget(std::move(view));
    }

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
    m_classesManager->RegisterBlockClass(std::make_shared<ComparatorBlockClass>());

    InitializeSidePanel();

    InitializeTools();

    auto scene_id = CreateNewScene();
    m_current_scene_id = {scene_id, m_sceneComponents[scene_id].manager->GetMainSubSceneId()};

    AddInitialNodes_forScene(*m_sceneComponents[m_current_scene_id->manager].manager);

    CreateSceneForSubsystem(*m_current_scene_id);
    m_tabbedView->SetCurrentTabIndex(0);

    m_tab_change_notifier = std::make_unique<TabChangeNotifier>(*this);
    m_tabbedView->Attach(*m_tab_change_notifier);


    GetToolBar()->SetFocusProxy(m_tabbedView);
    SetFocus(GetCenterWidget());
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
        [](const SimulationEvent::RequestedBadScene& e)
        {
            SDL_Log("requested bad scene with id: %d", e.subscene_id.value);
        },
        [&](const SimulationEvent::Success&)
        {
            SDL_Log("Success!");
            m_sceneComponents[m_current_scene_id->manager].manager->SetLastSimulationResults(std::move(m_sim_mgr.GetLastSimulationResults()));
            for (auto&& block_result : m_sceneComponents[m_current_scene_id->manager].manager->GetLastSimulationResults())
            {
                auto session_id = m_sim_mgr.GetLastSimulationSessionId();
                if (!session_id)
                {
                    SDL_Log("No Simulation session Id!");
                    return;
                }
                auto it = std::find_if(m_sceneComponents.begin(), m_sceneComponents.end(), [&](const auto& component) { return component.first == session_id; });
                if (it == m_sceneComponents.end())
                {
                    return;
                }

                auto graphicsObjectsManager = it->second.manager->GetManager(it->second.manager->GetMainSubSceneId());
                auto&& block_it = graphicsObjectsManager->getBlocksRegistry().find(block_result.id);
                if (block_it != graphicsObjectsManager->getBlocksRegistry().end())
                {
                    auto& objects_dialogs = it->second.manager->GetDialogs(it->second.manager->GetMainSubSceneId());
                    auto dialog_it = objects_dialogs.find(block_it->second);
                    if (dialog_it != objects_dialogs.end() && dialog_it->second.type == DialogType::BlockDialog && dialog_it->second.dialog.isAlive())
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
            m_sim_mgr.GetSimulationSettings(), SDL_FRect{ 100.0f,100.0f,0.0f,0.0f }, this);
        m_settings_dialog = dialog->GetMIHandlePtr();
        auto dialog_ptr = dialog.get();
        AddNormalDialog(std::move(dialog));
        SetFocus(dialog_ptr);
    }
    else
    {
        auto* dialog = static_cast<SimulationSettingsDialog*>(m_settings_dialog.GetObjectPtr());
        const auto& rect = dialog->GetRect();
        dialog->SetRect({ 100, 100, rect.w, rect.h });
        BumpDialogToTop(dialog);
    }
}

MI::ClickEvent node::MainNodeScene::OnRMBUp(MouseButtonEvent& e)
{
    if (Scene::OnRMBUp(e) != MI::ClickEvent::NONE)
    {
        return MI::ClickEvent::CLICKED;
    }
    //std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this);
    //this->ShowContextMenu(std::move(menu), {p.x, p.y});
    return MI::ClickEvent::NONE;
}

