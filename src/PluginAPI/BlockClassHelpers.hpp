#pragma once

#include "PluginAPI/BlockTemplate.h"
#include "BlockClass.hpp"
#include <optional>

namespace node
{
namespace helper
{

	static BlockTemplate CBlockTemplateToCPP(const CBlockTemplate& block_template)
	{
		using namespace node::model;


		BlockTemplate temp;
		if (auto* ptr = block_template.category.data)
		{
			temp.category = std::string{ ptr, ptr + block_template.category.size };
		}
		if (auto* ptr = block_template.template_name.data)
		{
			temp.template_name = std::string{ ptr, ptr + block_template.template_name.size };
		}
		if (auto* ptr = block_template.styler_name.data)
		{
			temp.styler_name = std::string{ ptr, ptr + block_template.styler_name.size };
		}
		if (block_template.block_type == CBlockTypeFunctional)
		{
			auto& func_data = block_template.functional_data;
			temp.data.data = FunctionalBlockData{};
			FunctionalBlockData& functinal_data = *temp.data.GetFunctionalData();
			if (auto* ptr = func_data.block_class.data)
			{
				functinal_data.block_class = std::string{ ptr, ptr + func_data.block_class.size };
			}
			for (size_t i = 0; i < func_data.properties.size; i++)
			{
				auto& func_prop = func_data.properties.data[i];
				std::string name;
				if (auto* ptr = func_prop.name.data)
				{
					name = std::string{ ptr, ptr + func_prop.name.size };
				}
				switch (func_prop.prop_type)
				{

				case CBlockPropertyTypeBool:
				{
					functinal_data.properties.push_back(*BlockProperty::Create(std::move(name), BlockPropertyType::Boolean, static_cast<bool>(func_prop.bool_val)));
					continue;
				}
				case CBlockPropertyTypeFloat:
				{
					functinal_data.properties.push_back(*BlockProperty::Create(std::move(name), BlockPropertyType::FloatNumber, func_prop.float_val));
					continue;
				}
				case CBlockPropertyTypeInt:
				{
					functinal_data.properties.push_back(*BlockProperty::Create(std::move(name), BlockPropertyType::Integer, func_prop.int_val));
					continue;
				}
				case CBlockPropertyTypeUInt:
				{
					functinal_data.properties.push_back(*BlockProperty::Create(std::move(name), BlockPropertyType::UnsignedInteger, func_prop.uint_val));
					continue;
				}
				case CBlockPropertyTypeStr:
				{
					std::string prop_value;
					if (auto* ptr = func_prop.str_val.data)
					{
						prop_value = std::string{ ptr, ptr + func_prop.str_val.size };
					}
					functinal_data.properties.push_back(*BlockProperty::Create(std::move(name), BlockPropertyType::String, std::move(prop_value)));
					continue;
				}
				}
			}
		}

		for (size_t i = 0; i < block_template.style_properties.size; i++)
		{
			auto& prop = block_template.style_properties.data[i];
			std::string name;
			if (auto* ptr = prop.name.data)
			{
				name = std::string{ ptr, ptr + prop.name.size };
			}
			std::string value;
			if (auto* ptr = prop.value.data)
			{
				value = std::string{ ptr, ptr + prop.value.size };
			}
			temp.style_properties.properties.emplace(std::move(name), std::move(value));
		}

		return temp;
	}

}
}
