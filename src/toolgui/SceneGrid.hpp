#pragma once

#include "toolgui/Widget.hpp"
#include "toolgui/ButtonWidget.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class SidePanel;
struct PanelCloseRequest;

class SceneGrid : public Widget, public MultiObserver<PanelCloseRequest>
{
public:
	SceneGrid(const WidgetSize& size, Widget* parent);
	~SceneGrid() override;

	void SetMainWidget(std::unique_ptr<Widget> widget);
	Widget* GetMainWidget() { return m_main_widget.get(); }

	void SetSidePanel(std::unique_ptr<SidePanel> widget);
	void SetBotPanel(std::unique_ptr<SidePanel> widget);

	SidePanel* GetSidePanel() { return m_side_panel.get(); }
	SidePanel* GetBotPanel() { return m_bot_panel.get(); }

	void OpenBotPanel();

	void OnNotify(PanelCloseRequest& event) override;
protected:
	void OnSetSize(const WidgetSize& size) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseMove(MouseHoverEvent& e) override;

private:
	SDL_FPoint GetMainWidgetPosition() const;
	WidgetSize GetMainWidgetSize() const;

	SDL_FPoint GetSidePanelPosition() const;
	SDL_FPoint GetBotPanelPosition() const;

	float GetSidePanelExpectedHeight() const;
	float GetBotPanelExpectedWidth() const;

	SDL_FPoint GetSidePanelBtnPosition() const;
	SDL_FPoint GetBotPanelBtnPosition() const;

	void OnOpenSidePanelPressed();
	void OnOpenBotPanelPressed();

	void OnCloseSidePanelPressed();
	void OnCloseBotPanelPressed();

	enum class PanelState
	{
		openning,
		opened,
		closing,
		closed,
	};

	struct PanelVisibilityData
	{
		float visibility_progress = 0;
		PanelState panel_state = PanelState::closed;
		uint64_t last_action_time = 0;
		int64_t updateTaskId = 0;
	};

	void UpdatePanelProgress(PanelVisibilityData& panel_visibility);
	void RepositionWidgets();

	enum class PanelsDragState
	{
		None,
		SidePanel,
		BotPanel,
	};

	static constexpr float widgets_margin = 5;
	static constexpr double TICKS_PER_SECOND = 1000;
	static constexpr double TRANSITION_TIME = 0.25;

	static constexpr float min_side_panel_wdith = 100;
	static constexpr float min_bot_panel_height = 100;

	std::unique_ptr<Widget> m_main_widget;
	std::unique_ptr<SidePanel> m_side_panel;
	std::unique_ptr<SidePanel> m_bot_panel;
	ButtonWidget m_open_sidepanel_btn;
	ButtonWidget m_open_botpanel_btn;

	PanelVisibilityData m_side_panel_visibility;
	PanelVisibilityData m_bot_panel_visibility;
	PanelsDragState m_drag_state;
	float m_side_panel_preferred_width = 0;
	float m_bot_panel_preferred_height = 0;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};
}