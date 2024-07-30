#pragma once

#include "DraggableObject.hpp"
#include "BlockSocketObject.hpp"
#include <vector>
#include <variant>
#include <list>

namespace node {

struct NetObject;
class NetSegment;
class NetNode;
class IGraphicsScene;

enum class NetOrientation
{
	Vertical,
	Horizontal
};

enum class NetSide
{
	North,
	South,
	East,
	West
};

constexpr int NET_NODE_OBJECT_Z = 100;
class GRAPHICSSCENE_API NetNode : public GraphicsObject
{
public:
	explicit NetNode(const SDL_Point& center, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	const SDL_Point& getCenter() noexcept { return m_centerPoint; }
	void setSegment(NetSegment* segment, NetSide side);
	NetSegment* getSegment(NetSide side)
	{
		switch (side)
		{
		case NetSide::North:
			return m_northSegment;
		case NetSide::South:
			return m_southSegment;
		case NetSide::East:
			return m_eastSegment;
		case NetSide::West:
			return m_westSegment;
		}
		return nullptr;
	}
	void setCenter(const model::Point& point) noexcept { SetSpaceOrigin({ point.x - m_width / 2, point.y - m_height / 2 }); }
	void UpdateConnectedSegments();
	void SetConnectedSocket(BlockSocketObject* socket);
	BlockSocketObject* GetConnectedSocket() noexcept;
	uint8_t GetConnectedSegmentsCount();
	void ClearSegment(const NetSegment* segment);
protected:
	virtual MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
	virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	virtual void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnSetSpaceRect(const model::Rect& rect) override;
private:
	SDL_Point m_centerPoint;
	NetSegment* m_northSegment = nullptr;
	NetSegment* m_southSegment = nullptr;
	NetSegment* m_eastSegment = nullptr;
	NetSegment* m_westSegment = nullptr;
	BlockSocketObject* m_socket = nullptr;
	static constexpr int m_width = 10;
	static constexpr int m_height = 10;
	bool b_being_deleted = false;
};

constexpr int NET_SEGMENT_OBJECT_Z = 50;

class GRAPHICSSCENE_API NetSegment : public GraphicsObject
{
public:
	explicit NetSegment(const NetOrientation& orientation, 
	NetNode* startNode = nullptr, NetNode* endNode = nullptr, IGraphicsScene* scene = nullptr);
	virtual void Draw(SDL_Renderer* renderer) override;
	const NetOrientation& getOrientation() noexcept { return m_orientation; }
	NetNode* getStartNode() noexcept { return m_startNode; }
	NetNode* getEndNode() noexcept { return m_endNode; }
	void Connect(NetNode* start, NetNode* end, const NetOrientation& orientation);
	void Disconnect();
	void CalcRect();
	const NetOrientation& GetOrientation() noexcept { return m_orientation; }
	int GetWidth() const { return c_width; }
protected:
	virtual MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
	void OnMouseMove(const model::Point& current_mouse_point) override;
	virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	NetNode* m_startNode;
	NetNode* m_endNode;
	NetOrientation m_orientation{};
	static constexpr int c_width = 10;
	bool b_being_dragged = false;
	bool b_being_deleted = false;
};

struct GRAPHICSSCENE_API NetObject
{
	std::string net_name = "net1";
	std::vector<NetNode*> m_nodes;
	std::vector<NetSegment*> m_segments;
};

};