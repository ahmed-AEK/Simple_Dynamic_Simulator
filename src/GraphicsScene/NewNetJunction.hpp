#pragma once

#include "GraphicsScene/NewNet.hpp"

#include <array>

namespace node {

class NetSegment;

class GRAPHICSSCENE_API NewNetJunctionObject : public node::NewNetObject
{
public:
	static NewNetJunctionObject* TryCreate(NetSegment* startSegment, const SDL_Point& startPoint, GraphicsScene* scene);
	NewNetJunctionObject(NetNode* startNode, NetNode* endNode, GraphicsScene* scene, NetSegment* segment_to_fuse);
protected:
	virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
private:
	NetSegment* m_segmentToFuse = nullptr;
};

}