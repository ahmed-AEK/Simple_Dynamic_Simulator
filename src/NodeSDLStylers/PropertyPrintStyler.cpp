#include "PropertyPrintStyler.hpp"
#include "NodeModels/NodeScene.hpp"
#include "NodeModels/BlockData.hpp"

#include <algorithm>
#include <charconv>

static std::string GetPrintedBlockPropertyValue(const node::model::BlockModel& model, const node::model::FunctionalBlockData* data)
{
	using namespace node;
	auto it = model.GetStylerProperties().properties.find(PropertyPrintStyler::printed_key_text);
	if (it == model.GetStylerProperties().properties.end())
	{
		return {};
	}

	if (!data)
	{
		SDL_Log("invalid block data!");
		return "error!";
	}
	
	const auto& properties = data->properties;

	auto it2 = std::find_if(properties.begin(), properties.end(), [&](const model::BlockProperty& prop) { return prop.name == it->second; });
	if (it2 != properties.end())
	{
		return it2->to_string();
	}
	return {};
}


node::PropertyPrintStyler::PropertyPrintStyler(const model::BlockDataCRef& model, TTF_Font* font)
	:TextBlockStyler{GetPrintedBlockPropertyValue(model.block, model.GetFunctionalData()),font}
{
}

void node::PropertyPrintStyler::UpdateProperties(const model::BlockDataCRef& model)
{
	SetText(GetPrintedBlockPropertyValue(model.block, model.GetFunctionalData()));
}
