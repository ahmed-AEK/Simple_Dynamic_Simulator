#include "NetTool.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObject.hpp"
#include "GraphicsLogic/NewNetLogic.hpp"
#include "BlockSocketObject.hpp"
#include "NetObject.hpp"

MI::ClickEvent node::NetTool::OnObjectLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
{
    using namespace node::model;

	scene.ClearCurrentSelection();


    switch (object.GetObjectType())
    {
    case ObjectType::socket:
    {
        auto* socket = static_cast<BlockSocketObject*>(&object);
        if (!socket->GetConnectedNode())
        {
            auto new_logic = logic::NewNetLogic::CreateFromSocket(*socket, &scene, &manager);
            if (new_logic)
            {
                scene.SetGraphicsLogic(std::move(new_logic));
                return MI::ClickEvent::CLICKED;
            }
        }
        break;
    }
    case ObjectType::netSegment:
    {
        auto* segment = static_cast<NetSegment*>(&object);
        auto new_logic = logic::NewNetLogic::CreateFromSegment(*segment, e.point, &scene, &manager);
        if (new_logic)
        {
            scene.SetGraphicsLogic(std::move(new_logic));
            return MI::ClickEvent::CLICKED;
        }
        break;
    }
    default: break;
    }

    return MI::ClickEvent::CLICKED;
}

bool node::NetTool::IsObjectClickable(GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
{
    UNUSED_PARAM(scene); UNUSED_PARAM(manager);

    switch (object.GetObjectType())
    {
    case ObjectType::socket:
    {
        auto* socket = static_cast<BlockSocketObject*>(&object);
        if (!socket->GetConnectedNode())
        {
            return true;
        }
        break;
    }
    case ObjectType::netSegment:
    {
        return true;
    }
    default: break;
    }
    return false;
}
