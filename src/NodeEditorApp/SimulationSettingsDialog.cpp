#include "SimulationSettingsDialog.hpp"
#include "SimulationManager.hpp"
#include "toolgui/DialogControls.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"
#include <charconv>
#include <boost/charconv.hpp>

node::SimulationSettingsDialog::SimulationSettingsDialog(
	std::function<void(const SimulationSettings&)> ok_callback, 
	const SimulationSettings& inital_value, const WidgetSize& size, Scene* parent)
	:Dialog{"Simulation Settings", size, parent}, m_ok_callback{std::move(ok_callback)}
{
	assert(parent);

	auto* title_font = parent->GetApp()->getFont(FontType::Title).get();
	AddControl(std::make_unique<DialogLabel>(std::vector<std::string>{"Simulation Time"}, 
		WidgetSize{100.0f,30.0f}, title_font, this));

	{
		char buffer[25]{};
		std::to_chars(std::begin(buffer), std::end(buffer), inital_value.t_start);
		std::string initial_value{ buffer };
		auto ptr = std::make_unique<PropertyEditControl>("Start Time", 200, std::move(initial_value), 
			WidgetSize{ 500.0f, 35.0f }, title_font, nullptr, this);
		m_t_start_lbl = ptr.get();
		AddControl(std::move(ptr));
	}
	{
		char buffer[25]{};
		std::to_chars(std::begin(buffer), std::end(buffer), inital_value.t_end);
		std::string initial_value{ buffer };
		auto ptr = std::make_unique<PropertyEditControl>("End Time", 200, std::move(initial_value), 
			WidgetSize{ 500.0f, 35.0f }, title_font, nullptr, this);
		m_t_end_lbl = ptr.get();
		AddControl(std::move(ptr));
	}
	{
		char buffer[25]{};
		std::to_chars(std::begin(buffer), std::end(buffer), inital_value.max_step);
		std::string initial_value{ buffer };
		auto ptr = std::make_unique<PropertyEditControl>("Max Step", 200, std::move(initial_value), 
			WidgetSize{ 500.0f, 35.0f }, title_font, nullptr, this);
		m_max_step_lbl = ptr.get();
		AddControl(std::move(ptr));
	}
	AddButton("Ok", [this]() {this->TriggerOk(); });
	AddButton("Cancel", [this]() {this->TriggerClose(); });
}

void node::SimulationSettingsDialog::OnOk()
{
	if (m_ok_callback)
	{
		auto grabber = [](const std::string& value, double& result)-> bool
			{
				auto res = boost::charconv::from_chars(value.data(), value.data() + value.size(), result);
				if (res.ec != std::errc{} || res.ptr != value.data() + value.size())
				{
					return false;
				}
				return true;
			};
		double t_min = 0;
		double t_max = 10;
		double max_step = 0.01;
		if (!grabber(m_t_start_lbl->GetValue(), t_min))
		{
			m_logger.LogError("Bad value for Min!");
			return;
		}

		if (!grabber(m_t_end_lbl->GetValue(), t_max))
		{
			m_logger.LogError("Bad value for Max!");
			return;
		}

		if (!grabber(m_max_step_lbl->GetValue(), max_step))
		{
			m_logger.LogError("Bad value for Step!");
			return;
		}

		m_ok_callback(SimulationSettings{ t_min,t_max, max_step});
		m_logger.LogInfo("Simulator Settings Changed!");
		Dialog::OnOk();
	}

}
