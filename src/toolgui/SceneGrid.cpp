#include "SceneGrid.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/SidePanel.hpp"

static float easeOut(double val) { return static_cast<float>(2 * val - val * val); }

node::SceneGrid::SceneGrid(const WidgetSize& size, Widget* parent)
	:Widget{ size, parent }, m_open_sidepanel_btn{ WidgetSize{35,50}, "Open Side Panel", "assets/arrow_left.svg", [this]() { this->OnOpenSidePanelPressed(); }, this }
{
	m_open_sidepanel_btn.SetPosition(GetSidePanelBtnPosition());
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
	if (m_side_panel && m_side_panel != widget)
	{
		m_side_panel->SetParent(nullptr);
		m_side_panel->Detach(*this);
	}
	m_side_panel = std::move(widget);
	if (m_side_panel)
	{
		m_side_panel->SetParent(this);
		m_side_panel->SetPosition(GetSidePanelPosition());
		m_side_panel->SetSize(GetSidePanelSize());
		m_side_panel->Attach(*this);
	}
}

void node::SceneGrid::OnNotify(PanelCloseRequest& event)
{
	if (event.panel == m_side_panel.get())
	{
		OnCloseSidePanelPressed();
	}
}

void node::SceneGrid::OnSetSize(const WidgetSize& size)
{
	Widget::OnSetSize(size);
	if (m_main_widget)
	{
		m_main_widget->SetSize(GetMainWidgetSize());
	}
	if (m_side_panel)
	{
		m_side_panel->SetPosition(GetSidePanelPosition());
		m_side_panel->SetSize(GetSidePanelSize());
	}
	m_open_sidepanel_btn.SetPosition(GetSidePanelBtnPosition());
}

SDL_FPoint node::SceneGrid::GetMainWidgetPosition() const
{
	return {widgets_margin, 0};
}

node::WidgetSize node::SceneGrid::GetMainWidgetSize() const
{
	auto&& size = GetSize();
	float side_panel_width = 0;
	if (m_side_panel)
	{
		side_panel_width = m_side_panel->GetSize().w + widgets_margin;
	}
	return {size.w - 2 * widgets_margin - side_panel_width * easeOut(m_side_panel_visibility_progress) - easeOut(1- m_side_panel_visibility_progress) * 28, size.h - widgets_margin};
}

SDL_FPoint node::SceneGrid::GetSidePanelPosition() const
{
	float side_panel_width = 0;
	if (m_side_panel)
	{
		side_panel_width = m_side_panel->GetSize().w;
	}
	return { GetSize().w - (widgets_margin + side_panel_width) * easeOut(m_side_panel_visibility_progress), widgets_margin};
}

node::WidgetSize node::SceneGrid::GetSidePanelSize() const
{
	if (m_side_panel)
	{
		return { m_side_panel->GetSize().w, GetSize().h - 2 * widgets_margin};
	}
	
	return {100, GetSize().h - 2 * widgets_margin };
}

SDL_FPoint node::SceneGrid::GetSidePanelBtnPosition() const
{
	return {GetSize().w - 28 * easeOut(1 - m_side_panel_visibility_progress), 10};
}

void node::SceneGrid::OnOpenSidePanelPressed()
{
	SDL_Log("Open SidePanel Pressed!");
	m_last_action_time = SDL_GetTicks();
	if (!m_updateTaskId)
	{
		m_panel_state = PanelState::openning;
		m_updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this]() { this->UpdateSidePanelProgress(); }));
	}
}

void node::SceneGrid::OnCloseSidePanelPressed()
{
	SDL_Log("Close SidePanel Pressed!");
	m_last_action_time = SDL_GetTicks();
	if (!m_updateTaskId)
	{
		m_panel_state = PanelState::closing;
		m_updateTaskId = GetApp()->AddUpdateTask(UpdateTask::FromWidget(*this, [this]() { this->UpdateSidePanelProgress(); }));
	}
}

void node::SceneGrid::UpdateSidePanelProgress()
{
	if (m_panel_state == PanelState::openning)
	{
		auto passed_ticks = SDL_GetTicks() - m_last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		m_side_panel_visibility_progress = static_cast<float>(passed_time / TRANSITION_TIME);
		if (m_side_panel_visibility_progress >= 1)
		{
			m_side_panel_visibility_progress = 1;
			m_panel_state = PanelState::opened;
			GetApp()->RemoveUpdateTask(m_updateTaskId);
			m_updateTaskId = 0;
		}
		RepositionWidgets();
	}
	else if (m_panel_state == PanelState::closing)
	{
		auto passed_ticks = SDL_GetTicks() - m_last_action_time;
		auto passed_time = passed_ticks / TICKS_PER_SECOND;
		m_side_panel_visibility_progress = static_cast<float>(1 - passed_time / TRANSITION_TIME);
		if (m_side_panel_visibility_progress <= 0)
		{
			m_side_panel_visibility_progress = 0;
			m_panel_state = PanelState::closed;
			GetApp()->RemoveUpdateTask(m_updateTaskId);
			m_updateTaskId = 0;
		}
		RepositionWidgets();
	}
	else
	{
		assert(false); // we shouldn't be here!
		GetApp()->RemoveUpdateTask(m_updateTaskId);
		m_updateTaskId = 0;
	}
}

void node::SceneGrid::RepositionWidgets()
{
	if (m_main_widget)
	{
		m_main_widget->SetSize(GetMainWidgetSize());
	}
	if (m_side_panel)
	{
		m_side_panel->SetPosition(GetSidePanelPosition());
	}
	m_open_sidepanel_btn.SetPosition(GetSidePanelBtnPosition());
}
