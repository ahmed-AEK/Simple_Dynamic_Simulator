#pragma once

#include "toolgui/Widget.hpp"

#include <concepts>

namespace node
{

class ToolTipWidget;

namespace mixin
{
	template <typename T>
	class TooltipMixin
	{
	public:
		~TooltipMixin()
		{
			HideToolTip();
		}
		void ToolTipMouseIn(MI::MouseHoverEvent<Widget>& e);
		void ToolTipMouseOut();
		void ToolTipMouseMove(MI::MouseHoverEvent<Widget>& e);
		void SetToolTipDescription(std::string description);
	private:
		void InternalUpdateToolTip();
		void HideToolTip();

		T& Self() { return *static_cast<T*>(this); }
		const T& Self() const { return *static_cast<const T*>(this); }

		uint64_t m_last_action_time = 0;
		int64_t m_updateTaskId = 0;
		SDL_FPoint m_last_mouse_pos{};
		std::string m_description;
		HandlePtrS<ToolTipWidget, Widget> m_toolTipWidget;

	};
}
}