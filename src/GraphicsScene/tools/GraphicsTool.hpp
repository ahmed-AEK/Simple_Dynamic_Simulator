#pragma once
#include "toolgui/MouseInteractable.hpp"
#include "NodeModels/Utils.hpp"

namespace node
{
class GraphicsScene;
class GraphicsObjectsManager;

class GraphicsTool
{
public:
	explicit GraphicsTool(GraphicsScene* scene, GraphicsObjectsManager* manager) 
		: m_scene{ scene }, m_objects_manager{ manager } {}
	~GraphicsTool() = default;
	GraphicsScene* GetScene() const { return m_scene; }
	GraphicsObjectsManager* GetObjectsManager() const { return m_objects_manager; }
	bool IsCapturingMouse() const noexcept { return m_capturing_mouse; }
	virtual MI::ClickEvent OnLMBDown(const model::Point& p) { UNUSED_PARAM(p); return MI::ClickEvent::NONE; }
	virtual MI::ClickEvent OnLMBUp(const model::Point& p) { UNUSED_PARAM(p); return MI::ClickEvent::NONE; }
	virtual void OnMouseMove(const model::Point& p) { UNUSED_PARAM(p); }
	virtual void OnMouseEnter(const model::Point& p) { UNUSED_PARAM(p); }
	virtual void OnMouseLeave(const model::Point& p) { UNUSED_PARAM(p); }
	virtual void OnDraw(SDL_Renderer* renderer) { UNUSED_PARAM(renderer); }
	virtual void OnStart() {}; // when tool just started
	virtual void OnExit() {}; // when tool is changed
	virtual void OnCancel() {}; // when escape is pressed
protected:
	bool m_capturing_mouse = false;
private:
	GraphicsScene* m_scene;
	GraphicsObjectsManager* m_objects_manager;
};
}