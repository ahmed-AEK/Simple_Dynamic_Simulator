#pragma once
#include "toolgui/MouseInteractable_interface.hpp"
#include "NodeModels/Utils.hpp"

namespace node
{
class GraphicsScene;
class GraphicsObjectsManager;
class GraphicsObject;

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
	virtual ~GraphicsTool() = default;
	bool IsCapturingMouse() const noexcept { return m_capturing_mouse; }
	virtual bool IsObjectClickable(GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
	{
		UNUSED_PARAM(scene); UNUSED_PARAM(manager);  UNUSED_PARAM(object); return false;
	}
	virtual MI::ClickEvent OnObjectLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
	{
		UNUSED_PARAM(e); UNUSED_PARAM(scene); UNUSED_PARAM(manager);  UNUSED_PARAM(object); return MI::ClickEvent::NONE;
	}
	virtual MI::ClickEvent OnSpaceLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager) 
	{
		UNUSED_PARAM(e); UNUSED_PARAM(scene); UNUSED_PARAM(manager);  return MI::ClickEvent::NONE;
	}
	virtual void OnStart() {}; // when tool just changed
	virtual void OnExit() {}; // when tool is changed
	virtual void OnCancel() {}; // when escape is pressed
protected:
	bool m_capturing_mouse = false;
private:
};
}