#include "NetTool.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObject.hpp"
#include "GraphicsLogic/NewNetLogic.hpp"
#include "BlockSocketObject.hpp"
#include "NetObject.hpp"

MI::ClickEvent node::NetTool::OnLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager)
{
    using namespace node::model;

	scene.ClearCurrentSelection();
	auto* obj = scene.GetObjectAt(e.point);
	if (!obj)
	{
		return MI::ClickEvent::NONE;
	}

    switch (obj->GetObjectType())
    {
    case ObjectType::socket:
    {
        auto* socket = static_cast<BlockSocketObject*>(obj);
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
        auto* segment = static_cast<NetSegment*>(obj);
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
