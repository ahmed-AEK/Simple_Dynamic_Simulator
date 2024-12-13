#include "SubSystemClass.hpp"

#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

static const std::vector<node::model::BlockProperty> ClassProperties{
	node::model::BlockProperty{"Storage", node::model::BlockPropertyType::String, std::string{"Inner"}},
	node::model::BlockProperty{"SubSceneId", node::model::BlockPropertyType::UnsignedInteger, uint64_t{0}},
};

static constexpr std::string_view Description = "A block that consists of many blocks";

node::SubSystemClass::SubSystemClass()
	:BlockClass{"SubSystem"}
{
}


const std::vector<node::model::BlockProperty>& node::SubSystemClass::GetDefaultClassProperties()
{
	return ClassProperties;
}

std::vector<node::model::BlockSocketModel::SocketType>
node::SubSystemClass::CalculateSockets(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	assert(ValidateClassProperties(properties));
	return {};
}

bool node::SubSystemClass::ValidateClassProperties(const std::vector<model::BlockProperty>& properties)
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
	}
	return true;
}


const std::string_view& node::SubSystemClass::GetDescription() const
{
	return Description;
}

node::BlockType node::SubSystemClass::GetBlockType(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return BlockType::Stateless;
}

node::BlockClass::GetFunctorResult node::SubSystemClass::GetFunctor(const std::vector<model::BlockProperty>& properties)
{
	UNUSED_PARAM(properties);
	return opt::NLEquation{ {0}, {1}, 
		opt::NLEquation::NLFunctor{
			[](std::span<const double> in, std::span<double> out) { out[0] = in[0]; }
		} 
	};
}
