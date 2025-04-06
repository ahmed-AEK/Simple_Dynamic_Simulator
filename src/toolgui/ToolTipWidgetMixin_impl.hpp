#pragma once

#include "toolgui/ToolTipWidgetMixin.hpp"
#include "toolgui/ToolTipWidget.hpp"
#include "toolgui/Application.hpp"
#include "SDL_Framework/SDL_Math.hpp"

template <typename T>
void node::mixin::TooltipMixin<T>::ToolTipMouseIn(MI::MouseHoverEvent<Widget>&)
{
	m_last_action_time = SDL_GetTicks();
	if (!m_updateTaskId && m_description.size())
	{
		m_updateTaskId = Self().GetApp()->AddUpdateTask(UpdateTask::FromWidget(Self(), [widget = &Self()]() { widget->InternalUpdateToolTip(); }));
	}
}

template <typename T>
void node::mixin::TooltipMixin<T>::ToolTipMouseOut()
{
	HideToolTip();
	if (m_updateTaskId)
	{
		Self().GetApp()->RemoveUpdateTask(m_updateTaskId);
		m_updateTaskId = 0;
	}
}

template <typename T>
void node::mixin::TooltipMixin<T>::ToolTipMouseMove(MI::MouseHoverEvent<Widget>& e)
{
	m_last_mouse_pos = e.point();
}

template <typename T>
void node::mixin::TooltipMixin<T>::SetToolTipDescription(std::string description)
{
	m_description = std::move(description);
}

template <typename T>
void node::mixin::TooltipMixin<T>::InternalUpdateToolTip()
{
	uint64_t current_time = SDL_GetTicks();
	if (current_time - m_last_action_time > 500 && m_updateTaskId)
	{
		constexpr float tooltip_y_offset = 15;
		auto toolTipWidget = ToolTipWidget::Create(
			Self().GetApp()->getFont().get(), m_description,
			&this->Self());
		m_toolTipWidget.reset(*toolTipWidget);
		Self().GetApp()->GetScene()->ShowToolTip(std::move(toolTipWidget), SDL_FPoint{ m_last_mouse_pos.x, m_last_mouse_pos.y} + Self().GetGlobalPosition(), tooltip_y_offset);
		Self().GetApp()->RemoveUpdateTask(m_updateTaskId);
		m_updateTaskId = 0;
	}
}

template <typename T>
void node::mixin::TooltipMixin<T>::HideToolTip()
{
	auto* app = Self().GetApp();
	if (!app)
	{
		return;
	}
	auto* scene = app->GetScene();
	if (scene && m_toolTipWidget)
	{
		scene->HideToolTip(m_toolTipWidget.GetObjectPtr());
		m_toolTipWidget = nullptr;
	}
}