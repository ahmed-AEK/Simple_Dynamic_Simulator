#pragma once

#include "toolgui/Widget.hpp"
#include "toolgui/ButtonWidget.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class SidePanel;
struct PanelCloseRequest;

class SceneGrid : public Widget, public SingleObserver<PanelCloseRequest>
{
public:
	SceneGrid(const WidgetSize& size, Widget* parent);
	~SceneGrid();

	void SetMainWidget(std::unique_ptr<Widget> widget);
	Widget* GetMainWidget() { return m_main_widget.get(); }

	void SetSidePanel(std::unique_ptr<SidePanel> widget);
	SidePanel* GetSidePanel() { return m_side_panel.get(); }

	void OnNotify(PanelCloseRequest& event);
protected:
	void OnSetSize(const WidgetSize& size) override;
private:
	SDL_FPoint GetMainWidgetPosition() const;
	WidgetSize GetMainWidgetSize() const;
	SDL_FPoint GetSidePanelPosition() const;
	WidgetSize GetSidePanelSize() const;
	SDL_FPoint GetSidePanelBtnPosition() const;
	void OnOpenSidePanelPressed();
	void OnCloseSidePanelPressed();
	void UpdateSidePanelProgress();
	void RepositionWidgets();

	enum class PanelState
	{
		openning,
		opened,
		closing,
		closed,
	};

	static constexpr float widgets_margin = 5;
	static constexpr double TICKS_PER_SECOND = 1000;
	static constexpr double TRANSITION_TIME = 0.25;

	std::unique_ptr<Widget> m_main_widget;
	std::unique_ptr<SidePanel> m_side_panel;
	ButtonWidget m_open_sidepanel_btn;
	float m_side_panel_visibility_progress = 0;
	PanelState m_panel_state = PanelState::closed;
	uint64_t m_last_action_time = 0;
	int64_t m_updateTaskId = 0;
};
}