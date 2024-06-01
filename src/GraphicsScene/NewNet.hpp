#pragma once

#include "GraphicsScene/GraphicsObject.hpp"

#include <array>

namespace node {

class NetNode;
class NetSegment;

enum class NewNetMode
{
	Horizontal,
	Vertical,
	Zmode,
	Hmode,
	Lmode
};

class GRAPHICSSCENE_API NewNetObject : public node::GraphicsObject
{
public:
	static NewNetObject* TryCreate(NetNode* endNode, GraphicsScene* scene);
	NewNetObject(NetNode* startNode, NetNode* endNode, GraphicsScene* scene);
	NewNetObject(std::array<NetNode*, 4> nodes, std::array<NetSegment*, 3> segments, GraphicsScene* scene);
	NetNode* GetStartNode() const { return p_startNode; }
	NetNode* GetEndNode() const { return p_endNode; }
protected:
	void OnMouseMove(const SDL_Point& current_mouse_point) override;
	virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
	virtual void Draw(SDL_Renderer* renderer) override;
private:
	void UpdateConnectedSegments();
	void UpdateToHorizontal();
	void UpdateToVertical();
	void UpdateToZmode();
	NetNode* p_startNode;
	NetNode* p_endNode;
	std::vector<NetNode*> m_intermediateNodes;
	std::vector<NetSegment*> m_segments;
	NewNetMode m_mode{};
};

}