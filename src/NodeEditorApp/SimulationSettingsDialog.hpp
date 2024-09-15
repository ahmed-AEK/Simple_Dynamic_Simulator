#pragma once

#include "toolgui/Dialog.hpp"
#include <functional>

namespace node
{

struct SimulationSettings;
class PropertyEditControl;

class SimulationSettingsDialog: public Dialog
{
public:
	SimulationSettingsDialog(std::function<void(const SimulationSettings&)> ok_callback, const SimulationSettings& inital_value, const SDL_Rect& rect, Scene* parent);
	~SimulationSettingsDialog();
private:
	void OnOk() override;
	std::function<void(const SimulationSettings&)> m_ok_callback;
	PropertyEditControl* m_t_start_lbl = nullptr;
	PropertyEditControl* m_t_end_lbl = nullptr;
	PropertyEditControl* m_max_step_lbl = nullptr;
};

}