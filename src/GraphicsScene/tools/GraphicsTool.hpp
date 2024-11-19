#pragma once
#include "toolgui/MouseInteractable_interface.hpp"
#include "NodeModels/Utils.hpp"

namespace node
{
class GraphicsScene;
class GraphicsObjectsManager;

class GraphicsTool
{
public:

	struct MouseHoverEvent
	{
		model::Point point;
	};

	struct MouseButtonEvent
	{
		model::Point point;
		bool double_click = false;
	};

	GraphicsTool() {}
	~GraphicsTool() = default;
	bool IsCapturingMouse() const noexcept { return m_capturing_mouse; }
	virtual MI::ClickEvent OnLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager) 
	{
		UNUSED_PARAM(e); UNUSED_PARAM(scene); UNUSED_PARAM(manager);  return MI::ClickEvent::NONE;
	}
	virtual MI::ClickEvent OnLMBUp(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager)
	{
		UNUSED_PARAM(e); UNUSED_PARAM(scene); UNUSED_PARAM(manager); return MI::ClickEvent::NONE;
	}
	virtual void OnMouseMove(MouseHoverEvent& p, GraphicsScene& scene, GraphicsObjectsManager& manager) 
	{ 
		UNUSED_PARAM(p); UNUSED_PARAM(scene); UNUSED_PARAM(manager); 
	}
	virtual void OnMouseEnter(MouseHoverEvent& p, GraphicsScene& scene, GraphicsObjectsManager& manager) 
	{ 
		UNUSED_PARAM(p); UNUSED_PARAM(scene); UNUSED_PARAM(manager); 
	}
	virtual void OnMouseLeave(MouseHoverEvent& p, GraphicsScene& scene, GraphicsObjectsManager& manager) 
	{ 
		UNUSED_PARAM(p); UNUSED_PARAM(scene); UNUSED_PARAM(manager); 
	}
	virtual void OnDraw(SDL_Renderer* renderer) { UNUSED_PARAM(renderer); }
	virtual void OnStart() {}; // when tool just changed
	virtual void OnExit() {}; // when tool is changed
	virtual void OnCancel() {}; // when escape is pressed
protected:
	bool m_capturing_mouse = false;
private:
};
}