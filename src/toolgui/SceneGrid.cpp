#include "SceneGrid.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/SidePanel.hpp"

static float easeOut(double val) { return static_cast<float>(2 * val - val * val); }

node::SceneGrid::SceneGrid(const WidgetSize& size, Widget* parent)
	:Widget{ size, parent }, m_open_sidepanel_btn{ WidgetSize{35,50}, "Open Blocks Panel", "assets/arrow_left.svg", [this]() { this->OnOpenSidePanelPressed(); }, this },
	m_open_botpanel_btn{ WidgetSize{50,35}, "Open Log", "assets/arrow_up.svg", [this]() { this->OnOpenBotPanelPressed(); }, this }
{
	m_open_sidepanel_btn.SetPosition(GetSidePanelBtnPosition());
	m_open_botpanel_btn.SetPosition(GetBotPanelBtnPosition());
}

node::SceneGrid::~SceneGrid()
{
}

void node::SceneGrid::SetMainWidget(std::unique_ptr<Widget> widget)
{
	if (m_main_widget && m_main_widget != widget)
	{
		m_main_widget->SetParent(nullptr);
	}
	m_main_widget = std::move(widget);
	if (m_main_widget)
	{
		m_main_widget->SetParent(this);
		m_main_widget->SetSize(GetMainWidgetSize());
		m_main_widget->SetPosition(GetMainWidgetPosition());
	}
	SetFocusProxy(m_main_widget.get());
}

void node::SceneGrid::SetSidePanel(std::unique_ptr<SidePanel> widget)
{
	assert(widget != m_side_panel);
	if (m_side_panel && m_side_panel != widget)
	{
		m_side_panel->SetParent(nullptr);
		m_side_panel->Detach(*this);
	}
	m_side_panel = std::move(widget);
	if (m_side_panel)
	{
		m_side_panel_preferred_width = m_side_panel->GetSize().w;
		m_side_panel->SetParent(this);
		m_side_panel->Attach(*this);
	}
	RepositionWidgets();
}

void node::SceneGrid::SetBotPanel(std::unique_ptr<SidePanel> widget)
{
	assert(widget != m_bot_panel);
	if (m_bot_panel)
	{
		m_bot_panel->SetParent(nullptr);
		m_bot_panel->Detach(*this);
	}
	m_bot_panel = std::move(widget);
	if (m_bot_panel)
	{
		m_bot_panel_preferred_height = m_bot_panel->GetSize().h;
		m_bot_panel->SetParent(this);
		m_bot_panel->Attach(*this);
	}
	RepositionWidgets();
}

void node::SceneGrid::OnNotify(PanelCloseRequest& event)
{
	switch (event.side)
	{
	case PanelSide::right:
	{
		if (event.panel == m_side_panel.get())
		{
			OnCloseSidePanelPressed();
		}
		break;
	}
	case PanelSide::bottom:
	{
		if (event.panel == m_bot_panel.get())
		{
			OnCloseBotPanelPressed();
		}
		break;
	}
	}
}

float node::SceneGrid::GetSidePanelExpectedHeight() const
{
	float bot_panel_y_offset = 0;
	if (m_bot_panel)
	{
		bot_panel_y_offset = (m_bot_panel->GetSize().h + widgets_margin) * easeOut(m_bot_panel_visibility.visibility_progress);
	}
	return GetSize().h - 2 * widgets_margin - bot_panel_y_offset;
}

float node::SceneGrid::GetBotPanelExpectedWidth() const
{
	return GetSize().w - 2 * widgets_margin;
}

void node::SceneGrid::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	RepositionWidgets();
}

MI::ClickEvent node::SceneGrid::OnLMBDown(MouseButtonEvent& e)
{
	if (!m_main_widget)
	{
		return MI::ClickEvent{};
	}
	if (m_side_panel && m_side_panel_visibility.panel_state == PanelState::opened)
	{
		float start_x = m_main_widget->GetPosition().x + m_main_widget->GetSize().w;
		float start_y = m_main_widget->GetPosition().y;
		float height = m_main_widget->GetSize().h;
		float width = m_side_panel->GetPosition().x - start_x;
		SDL_FRect right_gap{start_x, start_y, width, height};
		SDL_FPoint mouse_point = e.point();
		if (SDL_PointInRectFloat(&mouse_point, &right_gap))
		{
			m_drag_state = PanelsDragState::SidePanel;
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	if (m_bot_panel && m_bot_panel_visibility.panel_state == PanelState::opened)
	{
		float start_x = widgets_margin;
		float start_y = m_main_widget->GetSize().h + m_main_widget->GetPosition().y;
		float height = m_bot_panel->GetPosition().y - start_y;
		float width = m_main_widget->GetSize().w;

		SDL_FRect bot_gap{ start_x, start_y, width, height };
		SDL_FPoint mouse_point = e.point();
		if (SDL_PointInRectFloat(&mouse_point, &bot_gap))
		{
			m_drag_state = PanelsDragState::BotPanel;
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	UNUSED_PARAM(e);
	return MI::ClickEvent{};
}

MI::ClickEvent node::SceneGrid::OnLMBUp(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	if (m_drag_state != PanelsDragState::None)
	{
		m_drag_state = PanelsDragState::None;
		return MI::ClickEvent::CAPTURE_END;
	}
	return MI::ClickEvent{};
}

void node::SceneGrid::OnMouseMove(MouseHoverEvent& e)
{
	if (m_drag_state == PanelsDragState::SidePanel)
	{
		m_side_panel_preferred_width = GetSize().w - e.point().x - widgets_margin;
		if (m_side_panel_preferred_width < min_side_panel_wdith)
		{
			m_side_panel_preferred_width = min_side_panel_wdith;
		}
		RepositionWidgets();
	}
	if (m_drag_state == PanelsDragState::BotPanel)
	{
		m_bot_panel_preferred_height = GetSize().h - e.point().y - widgets_margin;
		if (m_bot_panel_preferred_height < min_bot_panel_height)
		{
			m_bot_panel_preferred_height = min_bot_panel_height;
		}
		RepositionWidgets();
	}
}

SDL_FPoint node::SceneGrid::GetMainWidgetPosition() const
{
	return {widgets_margin, 0};
}

node::WidgetSize node::SceneGrid::GetMainWidgetSize() const
{
	auto&& size = GetSize();
	float side_panel_width = 0;
	float bot_panel_height = 0;
	if (m_side_panel)
	{
		side_panel_width = (m_side_panel->GetSize().w + widgets_margin) * easeOut(m_side_panel_visibility.visibility_progress) + easeOut(1 - m_side_panel_visibility.visibility_progress) * 28;
	}
	if (m_bot_panel)
	{
		bot_panel_height = (m_bot_panel->GetSize().h + widgets_margin) * easeOut(m_bot_panel_visibility.visibility_progress) + easeOut(1 - m_bot_panel_visibility.visibility_progress) * 28;
	}
	return {size.w - 2 * widgets_margin - side_panel_width, 
		size.h - 2* widgets_margin - bot_panel_height };
}

SDL_FPoint node::SceneGrid::GetSidePanelPosition() const
{
	float side_panel_width = 0;
	if (m_side_panel)
	{
		side_panel_width = m_side_panel->GetSize().w;
	}
	return { GetSize().w - (widgets_margin + side_panel_width) * easeOut(m_side_panel_visibility.visibility_progress), widgets_margin};
}

SDL_FPoint node::SceneGrid::GetBotPanelPosition() const
{
	float bot_panel_height = 0;
	if (m_bot_panel)
	{
		bot_panel_height = m_bot_panel->GetSize().h;
	}
	return { widgets_margin, GetSize().h - (widgets_margin + bot_panel_height) * easeOut(m_bot_panel_visibility.visibility_progress) };
}

SDL_FPoint node::SceneGrid::GetSidePanelBtnPosition() const
{
	if (!m_side_panel)
	{
		return { GetSize().w, 10 };
	}
	return {GetSize().w - 28 * easeOut(1 - m_side_panel_visibility.visibility_progress), 10};
}

SDL_FPoint node::SceneGrid::GetBotPanelBtnPosition() const
{
	if (!m_bot_panel)
	{
		return { 10, GetSize().h };
	}
	return { 10, GetSize().h - 28 * easeOut(1 - m_bot_panel_visibility.visibility_progress) };
}

void node::SceneGrid::OnOpenSidePanelPressed()
{
	m_logger.LogDebug("Open SidePanel Pressed!");
	m_side_panel_visibility.last_action_time = SDL_GetTicks();
	if (!m_side_panel_visibility.updateTaskId)
	{
		m_side_panel_visibility.panel_state = PanelState::openning;
		m_side_panel_visibility.updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this, panel = &m_side_panel_visibility]() { this->UpdatePanelProgress(*panel); }));
	}
}

void node::SceneGrid::OnOpenBotPanelPressed()
{
	m_logger.LogDebug("Open BotPanel Pressed!");
	m_bot_panel_visibility.last_action_time = SDL_GetTicks();
	if (!m_bot_panel_visibility.updateTaskId)
	{
		m_bot_panel_visibility.panel_state = PanelState::openning;
		m_bot_panel_visibility.updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this, panel = &m_bot_panel_visibility]() { this->UpdatePanelProgress(*panel); }));
	}

}

void node::SceneGrid::OnCloseSidePanelPressed()
{
	m_logger.LogDebug("Close SidePanel Pressed!");
	m_side_panel_visibility.last_action_time = SDL_GetTicks();
	if (!m_side_panel_visibility.updateTaskId)
	{
		m_side_panel_visibility.panel_state = PanelState::closing;
		m_side_panel_visibility.updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this, panel = &m_side_panel_visibility]() { this->UpdatePanelProgress(*panel); }));
	}
}

void node::SceneGrid::OnCloseBotPanelPressed()
{
	m_logger.LogDebug("Close BotPanel Pressed!");
	m_bot_panel_visibility.last_action_time = SDL_GetTicks();
	if (!m_bot_panel_visibility.updateTaskId)
	{
		m_bot_panel_visibility.panel_state = PanelState::closing;
		m_bot_panel_visibility.updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this, panel = &m_bot_panel_visibility]() { this->UpdatePanelProgress(*panel); }));
	}
}

void node::SceneGrid::UpdatePanelProgress(PanelVisibilityData& panel_visibility)
{
	if (panel_visibility.panel_state == PanelState::openning)
	{
		auto passed_ticks = SDL_GetTicks() - panel_visibility.last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		panel_visibility.visibility_progress = static_cast<float>(passed_time / TRANSITION_TIME);
		if (panel_visibility.visibility_progress >= 1)
		{
			panel_visibility.visibility_progress = 1;
			panel_visibility.panel_state = PanelState::opened;
			GetApp()->RemoveUpdateTask(panel_visibility.updateTaskId);
			panel_visibility.updateTaskId = 0;
		}
		RepositionWidgets();
	}
	else if (panel_visibility.panel_state == PanelState::closing)
	{
		auto passed_ticks = SDL_GetTicks() - panel_visibility.last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		panel_visibility.visibility_progress = static_cast<float>(1 - passed_time / TRANSITION_TIME);
		if (panel_visibility.visibility_progress <= 0)
		{
			panel_visibility.visibility_progress = 0;
			panel_visibility.panel_state = PanelState::closed;
			GetApp()->RemoveUpdateTask(panel_visibility.updateTaskId);
			panel_visibility.updateTaskId = 0;
		}
		RepositionWidgets();
	}
	else
	{
		assert(false); // we shouldn't be here!
		GetApp()->RemoveUpdateTask(panel_visibility.updateTaskId);
		panel_visibility.updateTaskId = 0;
	}
}

void node::SceneGrid::RepositionWidgets()
{
	if (m_bot_panel)
	{
		m_bot_panel->SetSize({ GetBotPanelExpectedWidth(), std::min(GetSize().h - 100, m_bot_panel_preferred_height) });
		m_bot_panel->SetPosition(GetBotPanelPosition());
	}
	if (m_side_panel)
	{
		m_side_panel->SetSize({ std::min(GetSize().w - 200, m_side_panel_preferred_width), GetSidePanelExpectedHeight() });
		m_side_panel->SetPosition(GetSidePanelPosition());
	}
	if (m_main_widget)
	{
		m_main_widget->SetSize(GetMainWidgetSize());
	}
	m_open_sidepanel_btn.SetPosition(GetSidePanelBtnPosition());
	m_open_botpanel_btn.SetPosition(GetBotPanelBtnPosition());
}
