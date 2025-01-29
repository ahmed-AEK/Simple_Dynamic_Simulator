#include "ComparatorClass.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"threshold", node::model::BlockPropertyType::FloatNumber, 0.0 },
	node::model::BlockProperty{"Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 },
};

static constexpr std::string_view Description = "Output = input > threshold ? 1 : 0";
static constexpr double high_value = 1;
static constexpr double low_value = 0;

node::ComparatorBlockClass::ComparatorBlockClass()
	:BlockClass{"Comparator"}
{
}

const std::vector<node::model::BlockProperty>& node::ComparatorBlockClass::GetDefaultClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType> node::ComparatorBlockClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return {
		node::model::BlockSocketModel::SocketType::input,
		node::model::BlockSocketModel::SocketType::output
	};
}

const std::string_view& node::ComparatorBlockClass::GetDescription() const
{
	return Description;
}

bool node::ComparatorBlockClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
{
	if (properties.size() != ClassProperties.size())
	{
		return false;
	}
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].name != ClassProperties[i].name)
		{
			return false;
		}
		if (properties[i].type != ClassProperties[i].type)
		{
			return false;
		}
		if (!std::holds_alternative<double>(properties[i].prop))
		{
			return false;
		}
	}
	return true;
}

node::BlockType node::ComparatorBlockClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateful;
}

node::BlockClass::GetFunctorResult node::ComparatorBlockClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{

	struct ComparatorOutputTransition
	{
		double trigger_time;
		double start_value;
		double end_time;
		bool rising;
	};

	assert(ValidateClassProperties(properties));
	double threshold = std::get<double>(properties[0].prop);
	double rise_time = std::get<double>(properties[1].prop);
	if (rise_time < 1e-9)
	{
		rise_time = 1e-9;
	}
	auto transition = std::make_shared<ComparatorOutputTransition>(0, 0, 0, false);
	auto ret = opt::NLStatefulEquation{
		{0},
		{1},
		opt::NLStatefulEquation::NLStatefulFunctor{[threshold, transition, rise_time](std::span<const double>, std::span<double> out, const double t, const opt::NLStatefulEquation& eq)
		{
			const auto& crossings = eq.GetZeroCrossings();
			assert(crossings.size());
			
			double end_value;
			if (crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above)
			{
				end_value = high_value;
			}
			else
			{
				end_value = low_value;
			}

			if (t >= transition->end_time)
			{
				out[0] = end_value;
			}
			else
			{
				double current_value = transition->start_value + (end_value - transition->start_value) * (t - transition->trigger_time) / rise_time;
				out[0] = current_value;
			}
			
		}},
		opt::NLStatefulEquation::NLStatefulUpdateFunctor{[](std::span<const double>, const double, opt::NLStatefulEquation&)
		{

		}
		},
		opt::NLStatefulEquation::NLStatefulCrossFunctor{[transition, rise_time](const double& t, size_t, opt::NLStatefulEquation& eq)
		{
			const auto& crossings = eq.GetZeroCrossings();

			double start_value = 0;
			if (crossings[0].last_value == opt::ZeroCrossDescriptor::Position::above)
			{
				start_value = high_value;
			}
			else
			{
				start_value = low_value;
			}

			if (t >= transition->end_time)
			{
				transition->trigger_time = t;
				transition->start_value = start_value;
				transition->end_time = t + rise_time;
				transition->rising = crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above;
				double end_value = transition->rising ? high_value : low_value;
				if (start_value != end_value)
				{
					// create transition
					eq.GetEvent()->set = false;
					eq.GetEvent()->t = transition->end_time;
				}
			}
			else
			{
				// transition in progress
				double old_end_value = start_value;
				start_value = transition->start_value + (old_end_value - transition->start_value) * (t - transition->trigger_time) / rise_time;
				transition->start_value = start_value;
				transition->trigger_time = t;
				transition->rising = crossings[0].current_value == opt::ZeroCrossDescriptor::Position::above;
				double end_value = transition->rising ? high_value : low_value;
				transition->end_time = t + std::abs(end_value - transition->start_value) / (high_value - low_value) * rise_time;
				if (transition->end_time > t)
				{
					// push end_time away anway
					eq.GetEvent()->set = false;
					eq.GetEvent()->t = transition->end_time;
				}
			}
		}
		},
		opt::NLStatefulEquation::NLStatefulEventFunctor{[](const double&, opt::NLStatefulEquation&)
		{

		}
		}
	};
	ret.GetZeroCrossings().push_back({
		threshold,
		0,
		opt::ZeroCrossDescriptor::CrossType::both,
		opt::ZeroCrossDescriptor::Position::undefined,
		opt::ZeroCrossDescriptor::Position::undefined,
		});
	ret.GetEvent() = opt::NLStatefulEquation::EquationEvent{0, true};
	return ret;
}



