#include "NetTool.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObject.hpp"
#include "GraphicsLogic/NewNetLogic.hpp"
#include "BlockSocketObject.hpp"
#include "NetObject.hpp"

MI::ClickEvent node::NetTool::OnLMBDown(const model::Point& p)
{
    using namespace node::model;

	GetScene()->ClearCurrentSelection();
	auto* obj = GetScene()->GetObjectAt(p);
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
            auto new_logic = logic::NewNetLogic::CreateFromSocket(*socket, GetScene(), GetObjectsManager());
            if (new_logic)
            {
                GetScene()->SetGraphicsLogic(std::move(new_logic));
                return MI::ClickEvent::CLICKED;
            }
        }
        break;
    }
    case ObjectType::netSegment:
    {
        auto* segment = static_cast<NetSegment*>(obj);
        if (segment->GetOrientation() == NetSegmentOrientation::vertical)
        {
            auto new_logic = logic::NewNetLogic::CreateFromSegment(*segment, p, GetScene(), GetObjectsManager());
            if (new_logic)
            {
                GetScene()->SetGraphicsLogic(std::move(new_logic));
                return MI::ClickEvent::CLICKED;
            }
        }
        break;
    }
    default: break;
    }

    return MI::ClickEvent::CLICKED;
}
