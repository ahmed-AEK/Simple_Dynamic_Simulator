#include "PropertyPrintStyler.hpp"

#include <algorithm>
#include <charconv>

static std::string GetPrintedBlockPropertyValue(const node::model::BlockModel& model)
{
	using namespace node;
	auto it = model.GetStylerProperties().properties.find(PropertyPrintStyler::printed_key_text);
	if (it == model.GetStylerProperties().properties.end())
	{
		return {};
	}
	const auto& properties = model.GetProperties();
	auto it2 = std::find_if(properties.begin(), properties.end(), [&](const model::BlockProperty& prop) { return prop.name == it->second; });
	if (it2 != properties.end())
	{
		return it2->to_string();
	}
	return {};
}

std::unique_ptr<node::PropertyPrintStyler> node::PropertyPrintStyler::Create(const model::BlockModel& model, TTF_Font* font)
{
	using namespace node;
	return std::make_unique<PropertyPrintStyler>(GetPrintedBlockPropertyValue(model), font);
}

node::PropertyPrintStyler::PropertyPrintStyler(std::string name, TTF_Font* font)
	:TextBlockStyler{std::move(name),font}
{
}

void node::PropertyPrintStyler::UpdateProperties(const model::BlockModel& model)
{
	SetText(GetPrintedBlockPropertyValue(model));
}
