#include "MainNodeScene.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/SidePanel.hpp"
#include "toolgui/ToolBar.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/Dialog.hpp"
#include "toolgui/SceneGrid.hpp"
#include "toolgui/ScrollView.hpp"
#include "toolgui/LogView.hpp"

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
#include "GraphicsScene/tools/GraphicsToolsHandler.hpp"
#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

#include "PluginAPI/BlockClassesManager.hpp"

#include "PluginAPI/BlockDialog.hpp"

#include "BlockPalette/BlockPalette.hpp"
#include "BlockPalette/PaletteProvider.hpp"

#include "NodeSDLStylers/BlockStylerFactory.hpp"
#include "NodeSDLStylers/SVGBlockStyler.hpp"
#include "NodeSDLStylers/GainBlockStyler.hpp"
#include "NodeSDLStylers/PropertyPrintStyler.hpp"

#include "BuiltinClasses/BuiltinClassesPlugin.hpp"
#include "LuaPlugin/LuaRuntime.hpp"
#include "LuaPlugin/LuaStandaloneStatefulEqnClass.hpp"

#include "NodeEditorApp/SimulatorRunner.hpp"
#include "NodeEditorApp/BlockPropertiesDialog.hpp"
#include "NodeEditorApp/SimulationSettingsDialog.hpp"
#include "NodeEditorApp/AboutDialog.hpp"
#include "NodeEditorApp/NewSceneDialog.hpp"
#include "NodeEditorApp/SceneManager.hpp"
#include "NodeEditorApp/PluginsManager.hpp"

#include "SceneLoader/SceneLoader.hpp"
#include <filesystem>
#include <limits>
#include "toolgui/TabbedView.hpp"

#define USE_SDL_DIALOGS 1

static ColorPalette GetLightPalette()
{
    static ColorPalette palette =  ColorPalette{ ColorTable{std::initializer_list<ColorTable::ColorEntry>{
            {ColorRole::frame_background, 255, 255, 255},
            {ColorRole::frame_outline, 0, 0, 0},
            {ColorRole::frame_background_alternate, 235, 235, 235},
            {ColorRole::frame_outline_alternate, 204, 204, 204},
            {ColorRole::dialog_background, 250, 250, 250},
            {ColorRole::dialog_outline, 0, 0, 0},
            {ColorRole::dialog_title_background, 255, 255, 255},
            {ColorRole::text_normal, 50, 50, 50},
            {ColorRole::btn_normal, 255, 255, 255},
            {ColorRole::btn_disable, 190, 190, 190},
            {ColorRole::btn_hover, 235, 235, 235},
            {ColorRole::btn_outline, 210, 210, 210},
            {ColorRole::scrollbar, 50, 50, 50},
            {ColorRole::red_close, 255, 60, 60},
            {ColorRole::red_close_clicked, 180, 60, 60},
            {ColorRole::blue_select, 50, 153, 255},
            {ColorRole::block_outline, 0, 0, 0},
            {ColorRole::block_outline_selected, 255, 165, 0},
            {ColorRole::block_background, 220, 220, 220},
            {ColorRole::input_socket, 0, 255, 0},
            {ColorRole::output_socket, 0, 0, 255},
            {ColorRole::inout_socket, 255, 0, 0},
            {ColorRole::netnode_normal, 0, 0, 0},
            {ColorRole::netnode_selected, 40, 40, 40},
            {ColorRole::netsegment_normal, 100, 100, 100},
            {ColorRole::netsegment_selected, 255, 180, 0},
            {ColorRole::object_hover_outline, 0, 160, 0},
    }} };
    palette.SetDarkMode(false);
    return palette;
}

static ColorPalette GetDarkPalette()
{
    static ColorPalette palette = ColorPalette{ ColorTable{std::initializer_list<ColorTable::ColorEntry>{
            {ColorRole::frame_background, 48, 48, 48},
            {ColorRole::frame_outline, 124, 124, 124},
            {ColorRole::frame_background_alternate, 15, 15, 15},
            {ColorRole::frame_outline_alternate, 124, 124, 124},
            {ColorRole::dialog_background, 15, 15, 15},
            {ColorRole::dialog_outline, 124, 124, 124},
            {ColorRole::dialog_title_background, 48, 48, 48},
            {ColorRole::text_normal, 200, 200, 200},
            {ColorRole::btn_normal, 63, 63, 63},
            {ColorRole::btn_disable, 42, 42, 42},
            {ColorRole::btn_hover, 82, 82, 82},
            {ColorRole::btn_outline, 82, 82, 82},
            {ColorRole::scrollbar, 124, 124, 124},
            {ColorRole::red_close, 255, 60, 60},
            {ColorRole::red_close_clicked, 180, 60, 60},
            {ColorRole::blue_select, 13, 65, 171},
            {ColorRole::block_outline, 124, 124, 124},
            {ColorRole::block_outline_selected, 255, 165, 0},
            {ColorRole::block_background, 15, 15, 15},
            {ColorRole::input_socket, 0, 255, 0},
            {ColorRole::output_socket, 0, 0, 255},
            {ColorRole::inout_socket, 255, 0, 0},
            {ColorRole::netnode_normal, 10, 10, 10},
            {ColorRole::netnode_selected, 40, 40, 40},
            {ColorRole::netsegment_normal, 100, 100, 100},
            {ColorRole::netsegment_selected, 255, 180, 0},
            {ColorRole::object_hover_outline, 0, 255, 0},
    }} };
    palette.SetDarkMode();
    return palette;
}

static auto CalculateBlockSockets(std::span<const node::model::BlockProperty> properties, node::IBlockClass& block)
{
    node::CalculateSocketCallback cb;
    block.CalculateSockets(properties, cb);
    return std::move(cb.added_sockets);
}

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
                *model::BlockProperty::Create( "Slope",model::BlockPropertyType::FloatNumber, 1.0 )
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
                *model::BlockProperty::Create( "Multiplier",model::BlockPropertyType::FloatNumber, 1.0 )
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
                *model::BlockProperty::Create( "Inputs",model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1) )
            }
            });
        sceneModel->AddBlock(std::move(model));
    }
    /*
    {
        auto block_id = model::BlockId{ 6 };
        model::BlockModel model{ block_id, model::BlockType::Functional, model::Rect{ 200,10,100,100 } };
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0 } });
        model.AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 1 } });
        model.SetStyler("SVG Styler");
        model.SetStylerProperties(model::BlockStyleProperties{ {{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/lua_logo.svg"}} });
        sceneModel->GetFunctionalBlocksManager().SetDataForId(block_id,
            model::FunctionalBlockData{ "LuaStandaloneStatefulEqn" ,
            {
                *node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
                *node::model::BlockProperty::Create("Outputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
                *node::model::BlockProperty::Create("Code", node::model::BlockPropertyType::String, std::string{node::LuaStandaloneStatefulEqnClass::DEFAULT_CODE}),
            }
            });
        sceneModel->AddBlock(std::move(model));
    }
    */
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
    const auto& scene_manager = m_sceneComponents[m_current_scene_id->manager].manager;
    for (auto&& subscene : scene_manager->GetModels())
    {
        scene_models.push_back(subscene.second->GetModel());
    }
    m_logger.LogInfo("Starting Simulation!");
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
    m_logger.LogDebug("new Scene confirmed!");

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
LoadSceneAndSubscenes(node::loader::SceneLoader& db_conn, node::SubSceneId id, node::logging::Logger& logger)
{
    using namespace node;
    std::unordered_map<SubSceneId, std::shared_ptr<model::NodeSceneModel>> scenes;
    logger.LogDebug("Loading Scene %d", id.value);
    auto scene = db_conn.Load(id);
    if (!scene)
    {
        logger.LogError("Failed to Load Scene {}", id.value);
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
        auto child_scenes = LoadSceneAndSubscenes(db_conn, child_id, logger);
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
    m_logger.LogDebug("load Scene: {}", name);

    if (m_sim_mgr.IsSimulationRunning())
    {
        m_sim_mgr.StopSimulator();
    }
    m_sim_mgr.ClearLastSimulationReults();

    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    if (auto new_scenes = LoadSceneAndSubscenes(*connector.connector,SubSceneId{1}, unmove(logger(logging::LogCategory::Core))))
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
        m_logger.LogInfo("Loading Scene successful!");
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

static bool SaveSubSceneAndChildren(node::loader::SceneLoader& db_conn, node::SceneManager& components, node::SubSceneId id, node::SubSceneId parent_id, 
    node::logging::Logger& logger)
{
    using namespace node;
    logger.LogDebug("Saving Scene with id {}", id.value);
    auto ModelManager = components.GetModel(id);
    if (!ModelManager)
    {
        logger.LogError("Scene Model not found! {}", id.value);
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
            if (!SaveSubSceneAndChildren(db_conn, components, subsystemData.scene_id, id, logger))
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
    auto* db_connector = mgr->GetDBConnector();
    assert(db_connector);
    if (!db_connector)
    {
        return;
    }
    m_logger.LogDebug("scene Saved to {}", db_connector->db_path);
    if (db_connector->connector->Reset())
    {
        SaveSubSceneAndChildren(*db_connector->connector, 
            *m_sceneComponents[m_current_scene_id->manager].manager, mgr->GetMainSubSceneId(), SubSceneId{ 0 },
            unmove(logger(logging::LogCategory::Core)));
    }
    m_logger.LogInfo("Saving Scene Successful!");
}

void node::MainNodeScene::SaveScene(std::string name)
{
    if (!m_current_scene_id)
    {
        return;
    }

    auto& mgr = m_sceneComponents[m_current_scene_id->manager].manager;
    m_logger.LogDebug("scene Saved to {}", name);
    DBConnector connector{ std::move(name), nullptr };
    connector.connector = node::loader::MakeSqlLoader(connector.db_path);
    if (connector.connector->Reset() && 
        SaveSubSceneAndChildren(*connector.connector, 
            *m_sceneComponents[m_current_scene_id->manager].manager, mgr->GetMainSubSceneId(), SubSceneId{ 0 }, 
            unmove(logger(logging::LogCategory::Core))))
    {
        mgr->SetDBConnector(std::move(connector));
        m_logger.LogInfo("Saving Scene Successful!");

        auto scene_widget = mgr->GetManager(mgr->GetMainSubSceneId())->GetGraphicsScene();
        auto tab_idx = m_tabbedView->GetWidgetIndex(scene_widget);
        assert(tab_idx != m_tabbedView->npos);
        if (tab_idx != m_tabbedView->npos)
        {
            auto* db_connector = mgr->GetDBConnector();
            m_tabbedView->SetTabName(tab_idx, db_connector->connector->GetSceneName());
        }
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
    m_logger.LogDebug("scene Maybe Saved to {}", name);
    if (std::filesystem::is_regular_file(name))
    {
        auto dialog = std::make_unique<ConfirmOverwriteSaveSceneDialog>(std::move(name), WidgetSize{ 0.0f,0.0f }, this);
        dialog->SetPosition({ 100.0f,100.0f });
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
        m_logger.LogError("closing unavailable tab {}", static_cast<int>(tab_idx));
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
                if (auto idx = m_tabbedView->GetWidgetIndex(manager.second->GetGraphicsScene()); idx != -1)
                {
                    m_tabbedView->DeleteTab(idx);
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

void node::MainNodeScene::OpenBotPanel()
{
    if (m_scene_grid)
    {
        m_scene_grid->OpenBotPanel();
    }
}

node::LogView* node::MainNodeScene::GetLogView()
{
    return m_logView.GetObjectPtr();
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
    auto toolbar = std::make_unique<ToolBar>(WidgetSize{ 0.0f, ToolBar::height }, this);
    {
        auto new_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "New", [this]() {this->m_logger.LogDebug("New!"); this->NewScenePressed(); });
        new_btn->SetSVGPath("assets/new_file.svg");
        new_btn->SetDescription("New");
        toolbar->AddButton(std::move(new_btn));
    }
#if FILESYSTEM_SUPPORTED
    {
        auto load_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "Load", [this]() {this->m_logger.LogDebug("Load!"); this->LoadSceneButtonPressed(); });
        load_btn->SetSVGPath("assets/load_file.svg");
        load_btn->SetDescription("Load");
        toolbar->AddButton(std::move(load_btn));
    }
    {
        auto save_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "Save", [this]() {this->m_logger.LogDebug("Save!"); this->SaveSceneButtonPressed(); });
        save_btn->SetDescription("Save");
        save_btn->SetSVGPath("assets/save_file.svg");
        toolbar->AddButton(std::move(save_btn));
    }
#endif // FILESYSTEM_SUPPORTED
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
        auto arrow_btn = std::make_unique<ToolButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "A", m_toolsManager);
        arrow_btn->SetDescription("Arrow Tool");
        arrow_btn->SetSVGPath("assets/arrow.svg");
        toolbar->AddButton(std::move(arrow_btn));
    }
    {
        auto net_tool = std::make_unique<ToolButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "N", m_toolsManager);
        net_tool->SetDescription("Net Tool");
        net_tool->SetSVGPath("assets/net_tool.svg");
        toolbar->AddButton(std::move(net_tool));        
    }
    {
        auto delete_tool = std::make_unique<ToolButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "D", m_toolsManager);
        delete_tool->SetDescription("Delete Tool");
        delete_tool->SetSVGPath("assets/delete_tool.svg");
        toolbar->AddButton(std::move(delete_tool));
    }
    toolbar->AddSeparator();
    {
        auto undo_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "U",
            [this]() { this->m_logger.LogDebug("Undo"); this->OnUndo(); }, [this] {return this->CanUndo(); });
        undo_btn->SetDescription("Undo");
        undo_btn->SetSVGPath("assets/undo.svg");
        toolbar->AddButton(std::move(undo_btn));
    }
    {
        auto redo_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "R",
            [this]() { this->m_logger.LogDebug("Redo"); this->OnRedo(); }, [this] {return this->CanRedo(); });
        redo_btn->SetDescription("Redo");
        redo_btn->SetSVGPath("assets/redo.svg");
        toolbar->AddButton(std::move(redo_btn));
    }

    toolbar->AddSeparator();
    {
        auto prop_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "P",
            [this]() {this->m_logger.LogDebug("Properties!"); this->OpenPropertiesDialog(); });
        prop_btn->SetDescription("Properties");
        prop_btn->SetSVGPath("assets/properties.svg");
        toolbar->AddButton(std::move(prop_btn));
    }
    {
        auto run_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "R", [this]() {this->m_logger.LogDebug("Run!"); this->RunSimulator(); },
            [this]() { return !this->m_sim_mgr.IsSimulationRunning(); });
        run_btn->SetDescription("Run Simulation");
        run_btn->SetSVGPath("assets/run.svg");
        toolbar->AddButton(std::move(run_btn));
    }
    {
        auto stop_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "S", [this]() {this->m_logger.LogDebug("Stop!"); this->m_sim_mgr.StopSimulator(); },
            [this]() { return this->m_sim_mgr.IsSimulationRunning(); });
        stop_btn->SetDescription("Stop Simulation");
        stop_btn->SetSVGPath("assets/stop_sim.svg");
        toolbar->AddButton(std::move(stop_btn));
    }
    {
        auto settings_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "T", [this]() {this->m_logger.LogDebug("Settings!"); this->OnSettingsClicked(); });
        settings_btn->SetDescription("Settings");
        settings_btn->SetSVGPath("assets/settings.svg");
        toolbar->AddButton(std::move(settings_btn));
    }
    toolbar->AddSeparator();
    {
        auto dark_mode_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "K", [this]() {this->m_logger.LogDebug("DarkMode!"); this->OnDarkModeClicked(); });
        dark_mode_btn->SetDescription("Dark/Light Mode");
        dark_mode_btn->SetSVGPath("assets/dark_mode.svg");
        toolbar->AddButton(std::move(dark_mode_btn));
    }
    {
        auto about_btn = std::make_unique<ToolBarCommandButton>(WidgetSize{ ToolBarButton::width,ToolBarButton::height }, toolbar.get(), "I", [this]() {this->m_logger.LogDebug("About!"); this->OnAboutClicked(); });
        about_btn->SetDescription("About Software");
        about_btn->SetSVGPath("assets/about.svg");
        toolbar->AddButton(std::move(about_btn));
    }
    SetToolBar(std::move(toolbar));
    m_toolsManager->ChangeTool("A");
}

void node::MainNodeScene::InitializeBotPanel()
{
    auto* app_font = GetApp()->getFont(FontType::Label).get();

    auto bot_panel = std::make_unique<SidePanel>(PanelSide::bottom, app_font, WidgetSize{ GetSize().w, 100.0f }, this);

    auto log_view = std::make_unique<LogView>(WidgetSize{ 100,100 }, app_font, bot_panel.get());
    m_logView = HandlePtrS<LogView, Widget>{ *log_view };

    bot_panel->SetWidget(std::move(log_view));

    m_scene_grid->SetBotPanel(std::move(bot_panel));
}

void node::MainNodeScene::OnDarkModeClicked()
{
    m_dark_mode_active = !m_dark_mode_active;
    if (m_dark_mode_active)
    {
        SetColorPalette(GetDarkPalette());
        m_logger.LogDebug("activated dark mode!");
    }
    else
    {
        SetColorPalette(GetLightPalette());
        m_logger.LogDebug("activated light mode!");
    }
}

void node::MainNodeScene::InitializeSidePanel()
{
    auto sidePanel = std::make_unique<SidePanel>(SidePanel::PanelSide::right, GetApp()->getFont().get(),
        WidgetSize{ 250.0f,GetSize().h}, nullptr);


    auto&& palette_provider = std::make_shared<PaletteProvider>(m_classesManager, m_blockStylerFactory);
    m_palette_provider = palette_provider;
    auto* app_font = GetApp()->getFont(FontType::Title).get();

    sidePanel->SetWidget(std::make_unique<BlockPalette>(WidgetSize{200.0f,200.0f},
        std::move(palette_provider), app_font, sidePanel.get()));
    sidePanel->SetTitle("Block Palette");
    m_scene_grid->SetSidePanel(std::move(sidePanel));


}

namespace node
{

class BlockPropertiesUpdater final: public node::IBlockPropertiesUpdater
{
public:
    BlockPropertiesUpdater(model::BlockId block_id,
        std::weak_ptr<SceneModelManager> model_manager,
        BlockClassPtr block_class,
        std::weak_ptr<GraphicsObjectsManager> scene_manager)
        : m_block_id{ block_id }, m_model_manager{ model_manager }, m_block_class{ block_class },
        m_scene_manager{ scene_manager }
    {

    }
    std::optional<model::FunctionalBlockData> GetFunctionalBlockData() override
    {
        std::optional<model::FunctionalBlockData> ret;
        auto model_manager = m_model_manager.lock();
        auto* data = model_manager->GetModel().GetFunctionalBlocksManager().GetDataForId(m_block_id);
        if (data)
        {
            ret.emplace(*data);
        }
        return ret;
    }
    tl::expected<std::monostate, std::vector<ValidatePropertiesNotifier::PropertyError>> UpdateBlockProperties(std::span<const model::BlockProperty> new_properties) override
    {
        auto model_manager = m_model_manager.lock();
        if (!model_manager)
        {
            m_logger.LogDebug("Update Failed!");
            return tl::unexpected{ std::vector<ValidatePropertiesNotifier::PropertyError>{} };
        }
        auto scene_manager = m_scene_manager.lock();
        if (!scene_manager)
        {
            m_logger.LogDebug("Update Failed!");
            return tl::unexpected{ std::vector<ValidatePropertiesNotifier::PropertyError>{} };
        }
        auto block = model_manager->GetModel().GetBlockById(m_block_id);
        if (!block)
        {
            m_logger.LogDebug("Update Failed!");
            return tl::unexpected{ std::vector<ValidatePropertiesNotifier::PropertyError>{} };
        }

        auto block_data_ptr = model_manager->GetModel().GetFunctionalBlocksManager().GetDataForId(m_block_id);
        if (!block_data_ptr)
        {
            m_logger.LogDebug("Update Failed! data not found!");
            return tl::unexpected{ std::vector<ValidatePropertiesNotifier::PropertyError>{} };
        }

        assert(m_block_class.get());
        ValidatePropertiesNotifier notifier;
        if (!m_block_class->ValidateClassProperties(new_properties, notifier) || notifier.errors.size())
        {
            for (auto& error_text : notifier.errors)
            {
                m_logger.LogDebug("invalid Error index: {}", static_cast<int>(error_text.prop_idx));
            }
            m_logger.LogDebug("Update Failed class verification!");
            return tl::unexpected{ std::move(notifier.errors) };
        }

        bool renew_sockets = false;
        std::vector<model::BlockSocketModel> new_sockets;
        auto new_sockets_identifiers = CalculateBlockSockets(new_properties, *m_block_class);
        auto old_sockets = block->GetSockets();
        if (new_sockets_identifiers.size() != old_sockets.size())
        {
            renew_sockets = true;
        }
        else
        {
            for (size_t i = 0; i < new_sockets_identifiers.size(); i++)
            {
                if (new_sockets_identifiers[i].socket_type != old_sockets[i].GetType())
                {
                    renew_sockets = true;
                    break;
                }
            }
        }
        if (renew_sockets)
        {
            new_sockets.reserve(new_sockets_identifiers.size());
            for (size_t i = 0; i < new_sockets_identifiers.size(); i++)
            {
                new_sockets.push_back(model::BlockSocketModel{ new_sockets_identifiers[i].socket_type,model::SocketId{static_cast<model::id_int>(i)},
                    {}, {}, {}, new_sockets_identifiers[i].category });
            }

            auto&& block_registry = scene_manager->getBlocksRegistry();
            auto it = block_registry.find(m_block_id);
            assert(it != block_registry.end());
            if (it != block_registry.end())
            {
                it->second->GetStyler().PositionSockets(new_sockets, block->GetBounds(), block->GetOrienation());
            }
        }

        if (renew_sockets)
        {
            model_manager->ModifyBlockPropertiesAndSockets(m_block_id, std::vector<model::BlockProperty>{ new_properties.begin(), new_properties.end() }, std::move(new_sockets));
        }
        else
        {
            model_manager->ModifyBlockProperties(m_block_id, std::vector<model::BlockProperty>{ new_properties.begin(), new_properties.end() });
        }
        m_logger.LogDebug("Update Done!");
        return std::monostate{};
    }
private:
    logging::Logger m_logger = logger(logging::LogCategory::Core);
    model::BlockId m_block_id;
    std::weak_ptr<SceneModelManager> m_model_manager;
    BlockClassPtr m_block_class;
    std::weak_ptr<GraphicsObjectsManager> m_scene_manager;
};

}

void node::MainNodeScene::OpenPropertiesDialog()
{
    if (!m_current_scene_id)
    {
        return;
    }
    auto* centerWidget = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene)->GetGraphicsScene();
    auto&& selection = centerWidget->GetCurrentSelection();

    if (selection.size() == 0)
    {
        m_logger.LogError("No object is selected!");
        return;
    }

    if (selection.size() > 1)
    {
        m_logger.LogError("More than one object selected!");
        return;
    }

    auto object = selection[0].GetObjectPtr();
    if (!object || object->GetObjectType() != ObjectType::block)
    {
        m_logger.LogError("No block selected!");
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
            Dialog* dialog = it->second.dialog.GetObjectPtr();
            BumpDialogToTop(dialog);
            dialog->SetPosition({ 100.0f,100.0f});
            return;
        }
    }

    auto model_id = object.GetModelId();
    assert(model_id);
    if (!model_id)
    {
        m_logger.LogError("Block has no model!");
        return;
    }
    auto graphicsObjectsManager = m_sceneComponents[m_current_scene_id->manager].manager->GetManager(m_current_scene_id->subscene);
    auto* block = graphicsObjectsManager->GetSceneModel()->GetModel().GetBlockById(*model_id);
    if (!block)
    {
        m_logger.LogError("couldn't find the block model!");
        return;
    }
    auto* block_data = graphicsObjectsManager->GetSceneModel()->GetModel().GetFunctionalBlocksManager().GetDataForId(block->GetId());

    assert(m_classesManager);
    auto class_ptr = m_classesManager->GetBlockClassByName(block_data->block_class);
    auto updater = std::make_shared<BlockPropertiesUpdater>(block->GetId(), graphicsObjectsManager->GetSceneModel(),
        class_ptr, graphicsObjectsManager);
    auto dialog = std::make_unique<BlockPropertiesDialog>(updater,
        class_ptr, *block_data, WidgetSize{ 0.0f,0.0f }, this);
    dialog->SetPosition({ 100.0f,100.0f });
    objects_dialogs[static_cast<BlockObject*>(&object)] = DialogSlot{ HandlePtrS<Dialog,Widget>{*dialog}, DialogType::PropertiesDialog };
    auto dialog_ptr = dialog.get();
    AddNormalDialog(std::move(dialog));
    SetFocusLater(dialog_ptr);
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
            Dialog* dialog = it->second.dialog.GetObjectPtr();
            BumpDialogToTop(dialog);
            dialog->SetPosition({ 100.0f,100.0f });
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
            m_logger.LogError("class '{}' not found!", block_data->block_class);
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
            auto updater = std::make_shared<BlockPropertiesUpdater>(block_model.GetId(), graphicsObjectsManager->GetSceneModel(),
                class_ptr, graphicsObjectsManager);
            auto dialog = static_cast<BlockClass*>(class_ptr.get())->CreateBlockDialog(*this, updater,block_model, *block_data,sim_data);
            if (dialog)
            {
                objects_dialogs[&block] = DialogSlot{ HandlePtrS<Dialog,Widget>{*dialog}, DialogType::BlockDialog };
                auto dialog_ptr = dialog.get();
                dialog_ptr->SetPosition({ 100.0f,100.0f });
                AddNormalDialog(std::move(dialog));
                SetFocus(dialog_ptr);
            }
            else
            {
                OpenPropertiesDialog(block);
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
            m_logger.LogError("Can only handle local blocks");
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
                m_logger.LogError("Failed to create scene for subsystem {},{}",
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
        m_logger.LogDebug("Openning Dialog Not handled!");
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

    const auto* db_connector = m_sceneComponents[m_current_scene_id->manager].manager->GetDBConnector();
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
    auto scene_mgr = std::make_unique<SceneManager>(m_blockStylerFactory);
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
    std::unique_ptr<NodeGraphicsScene> gScene = std::make_unique<NodeGraphicsScene>(GetSize(), nullptr);
    auto graphicsObjectsManager = std::make_shared<GraphicsObjectsManager>(*gScene, m_blockStylerFactory, m_categories_styler);
    gScene->Attach(*graphicsObjectsManager);
    gScene->SetToolsManager(m_toolsManager);
    gScene->SetObjectsManager(graphicsObjectsManager);
    auto handler = std::make_shared<GraphicsToolsHandler>(*gScene, graphicsObjectsManager, m_toolsManager);
    gScene->SetTool(std::move(handler));

    scene_mgr.SetSubSceneManager(scene_id.subscene, graphicsObjectsManager);
    std::string scene_name;
    if (scene_id.subscene == scene_mgr.GetMainSubSceneId())
    {
        scene_name = scene_mgr.GetDBConnector() ? scene_mgr.GetDBConnector()->connector->GetSceneName() : "untitled";
    }
    else
    {
        scene_name = "Subsystem";
    }

    m_tabbedView->AddTab(scene_name, std::move(gScene));
    return true;
}

void node::MainNodeScene::OnUndo()
{
    m_logger.LogDebug("Undoed!");
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
    auto* scene = graphicsObjectsManager->GetGraphicsScene();
    if (scene)
    {
        scene->SetGraphicsLogic(nullptr);
    }
    graphicsObjectsManager->GetSceneModel()->Undo();
}

void node::MainNodeScene::OnRedo()
{
    m_logger.LogDebug("Redoed!");
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
    auto* scene = graphicsObjectsManager->GetGraphicsScene();
    if (scene)
    {
        scene->SetGraphicsLogic(nullptr);
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

node::MainNodeScene::MainNodeScene(const WidgetSize& size, node::Application* parent)
:Scene(size, parent)
{
}

namespace node
{

struct TabChangeNotifier : public SingleObserver<TabsChangeEvent>
{
    explicit TabChangeNotifier(MainNodeScene& scene) : m_scene{ &scene } {}
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

    m_categories_styler = std::make_shared<NetCategoriesStyleManager>();

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
        auto scene_grid = std::make_unique<SceneGrid>(WidgetSize{ 100,100 }, this);
        m_scene_grid.reset(*scene_grid);
        SetCenterWidget(std::move(scene_grid));
    }

    {
        std::unique_ptr<TabbedView> view = std::make_unique<TabbedView>(GetApp()->getFont(FontType::Label).get(), 
            WidgetSize{ 0,0 }, this);
        m_tabbedView.reset(*view);
        m_scene_grid->SetMainWidget(std::move(view));
    }

    m_classesManager = std::make_shared<BlockClassesManager>();

    InitializeSidePanel();
    InitializeBotPanel();

    m_plugins_manager = std::make_shared<PluginsManager>(m_palette_provider, m_classesManager);
    m_plugins_manager->AddRuntime(node::make_PluginRuntime<NativePluginsRuntime>());
    m_plugins_manager->AddRuntime(node::make_PluginRuntime<LuaRuntime>());

    InitializeTools();

    auto scene_id = CreateNewScene();
    m_current_scene_id = {scene_id, m_sceneComponents[scene_id].manager->GetMainSubSceneId()};

    AddInitialNodes_forScene(*m_sceneComponents[m_current_scene_id->manager].manager);

    CreateSceneForSubsystem(*m_current_scene_id);
    m_tabbedView->SetCurrentTabIndex(0);

    m_tab_change_notifier = std::make_unique<TabChangeNotifier>(*this);
    m_tabbedView->Attach(*m_tab_change_notifier);


    GetToolBar()->SetFocusProxy(m_scene_grid.GetObjectPtr());
    SetFocus(GetCenterWidget());
    if (SDL_GetSystemTheme() == SDL_SYSTEM_THEME_DARK)
    {
        m_dark_mode_active = true;
        SetColorPalette(GetDarkPalette());
    }
    else
    {
        SetColorPalette(GetLightPalette());
    }
}

node::MainNodeScene::~MainNodeScene() = default;

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

void node::MainNodeScene::OnSimulationEnd(const SimulationEvent& event)
{
    auto simulation_logger = logger(logging::LogCategory::Simulator);
    std::visit(overloaded{
        [&](const SimulationEvent::NetFloatingError&)
        {
            simulation_logger.LogError("Floating Net!");
        },
        [&](const SimulationEvent::Stopped&)
        {
            simulation_logger.LogInfo("Stopped!");
        },
        [&](const SimulationEvent::OutputSocketsConflict&)
        {
            simulation_logger.LogError("Sockets Conflict!");
        },
        [&](const SimulationEvent::FloatingInput&)
        {
           simulation_logger.LogError("Floating Input!");
        },
        [&](const SimulationEvent::RequestedBadScene& e)
        {
            simulation_logger.LogError("requested bad scene with id: {}", e.subscene_id.value);
        },
        [&](const SimulationEvent::SimulationError& e)
        {
            simulation_logger.LogError("Simulation Error: {}", e.error);
        },
        [&](const SimulationEvent::Success&)
        {
            simulation_logger.LogInfo("Simulation Succeed!");
            m_sceneComponents[m_current_scene_id->manager].manager->SetLastSimulationResults(std::move(m_sim_mgr.GetLastSimulationResults()));
            for (auto&& block_result : m_sceneComponents[m_current_scene_id->manager].manager->GetLastSimulationResults())
            {
                auto session_id = m_sim_mgr.GetLastSimulationSessionId();
                if (!session_id)
                {
                    m_logger.LogError("No Simulation session Id!");
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
    m_logger.LogDebug("simulation Ended!");
}

void node::MainNodeScene::OnSettingsClicked()
{
    if (!m_settings_dialog.isAlive())
    {
        auto dialog = std::make_unique<SimulationSettingsDialog>(
            [this](const auto& result) {this->m_sim_mgr.SetSimulationSettings(result); },
            m_sim_mgr.GetSimulationSettings(), WidgetSize{ 0.0f,0.0f }, this);
        dialog->SetPosition({ 100.0f, 100.0f });
        m_settings_dialog.reset(*dialog);
        auto dialog_ptr = dialog.get();
        AddNormalDialog(std::move(dialog));
        SetFocus(dialog_ptr);
    }
    else
    {
        auto* dialog = m_settings_dialog.GetObjectPtr();
        dialog->SetPosition({ 100, 100 });
        BumpDialogToTop(dialog);
    }
}

void node::MainNodeScene::OnAboutClicked()
{
    if (!m_about_dialog.isAlive())
    {
        auto dialog = std::make_unique<AboutDialog>(WidgetSize{ 0.0f,0.0f }, this);
        dialog->SetPosition({ 100.0f, 100.0f });
        m_about_dialog.reset(*dialog);
        auto dialog_ptr = dialog.get();
        AddNormalDialog(std::move(dialog));
        SetFocus(dialog_ptr);
    }
    else
    {
        auto* dialog = m_about_dialog.GetObjectPtr();
        dialog->SetPosition({ 100, 100 });
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

