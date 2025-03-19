#pragma once

#include "PluginAPI/BlockTemplate.h"
#include "BlockClass.hpp"
#include <optional>

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4505)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

namespace node
{
namespace helper
{
	namespace {
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
			if (block_template.block_type == CBlockTypeSubsystem)
			{
				temp.data = { SubsystemBlockData{{block_template.subsystem_data.URL.data, block_template.subsystem_data.URL.size}, SubSceneId{block_template.subsystem_data.scene_id} } };
			}
			if (block_template.block_type == CBlockTypePort)
			{
				temp.data = { PortBlockData{SocketId{block_template.port_data.socket_id}, SocketType(block_template.port_data.port_type)} };
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

		static CBlockProperty BlockPropertyToC(const model::BlockProperty& prop)
		{
			struct PropertyConverter
			{
				void operator()(const std::string& data) const {
					target.prop_type = CBlockPropertyTypeStr;
					target.str_val = { data.c_str(), data.size() };
				}
				void operator()(const double& data) const {
					target.prop_type = CBlockPropertyTypeFloat;
					target.float_val = data;
				}
				void operator()(const int64_t& data) const {
					target.prop_type = CBlockPropertyTypeInt;
					target.int_val = data;
				}
				void operator()(const uint64_t& data) const {
					target.prop_type = CBlockPropertyTypeUInt;
					target.uint_val = data;
				}
				void operator()(const bool& data) const {
					target.prop_type = CBlockPropertyTypeBool;
					target.bool_val = data;
				}
				CBlockProperty& target;
			};
			CBlockProperty ctarget{};
			std::visit(PropertyConverter{ ctarget }, prop.prop);
			ctarget.name = { prop.name.c_str(), prop.name.size() };
			return ctarget;
		}
		struct CBlockTemplateWrapper
		{
			CBlockTemplate cblock;
			std::vector<CBlockProperty> functional_block_properties;
			std::vector<CStylerProperty> styler_properties;
			operator CBlockTemplate() const
			{
				return cblock;
			}
		};

		static CBlockTemplateWrapper BlockTemplateToCBlock(const node::BlockTemplate& block)
		{
			struct BlockDataToCConverter
			{
				void operator()(const std::monostate&) {}
				void operator()(const model::FunctionalBlockData& data)
				{
					auto& cblock = target.cblock;
					cblock.block_type = CBlockTypeFunctional;
					cblock.functional_data.block_class = { data.block_class.c_str(), data.block_class.size() };
					cblock.functional_data.properties.size = data.properties.size();
					target.functional_block_properties.reserve(data.properties.size());
					for (const auto& property : data.properties)
					{
						target.functional_block_properties.push_back(BlockPropertyToC(property));
					}
					cblock.functional_data.properties.data = target.functional_block_properties.data();
				}
				void operator()(const model::SubsystemBlockData& data) {
					auto& cblock = target.cblock;
					cblock.block_type = CBlockTypeSubsystem;
					cblock.subsystem_data.scene_id = data.scene_id.value;
					cblock.subsystem_data.URL = { data.URL.c_str(), data.URL.size() };
				}
				void operator()(const model::PortBlockData& data) {
					auto& cblock = target.cblock;
					cblock.block_type = CBlockTypePort;
					cblock.port_data.port_type = static_cast<CSocketType>(data.port_type);
					cblock.port_data.socket_id = data.id.value;
				}

				CBlockTemplateWrapper& target;
			};

			CBlockTemplateWrapper block_wrapper{};
			auto& cblock = block_wrapper.cblock;
			cblock.category = { block.category.c_str(), block.category.size() };
			cblock.template_name = { block.template_name.c_str(), block.template_name.size() };
			cblock.styler_name = { block.styler_name.c_str(), block.styler_name.size() };
			std::visit(BlockDataToCConverter{ block_wrapper }, block.data.data);
			block_wrapper.styler_properties.reserve(block.style_properties.properties.size());
			for (const auto& [k, v] : block.style_properties.properties)
			{
				block_wrapper.styler_properties.push_back(CStylerProperty{ {k.c_str(), k.size()}, {v.c_str(), v.size()} });
			}
			cblock.style_properties = { block_wrapper.styler_properties.data(), block_wrapper.styler_properties.size() };
			return block_wrapper;

		}

		struct CBlockGroupWrapper
		{
			std::vector<helper::CBlockTemplateWrapper> wrappers;
			std::vector<CBlockTemplate> block_templates;
		};
		static CBlockGroupWrapper BlockTemplateGroupToC(std::span<const node::BlockTemplate> blocks)
		{
			CBlockGroupWrapper group;
			group.block_templates.reserve(blocks.size());
			for (const auto& block : blocks)
			{
				group.wrappers.push_back(helper::BlockTemplateToCBlock(block));
			}
			for (const auto& block : group.wrappers)
			{
				group.block_templates.push_back(block.cblock);
			}
			return group;
		}
	}
}
}
// Your function
#if defined(_MSC_VER)
#pragma warning( pop ) 
#else
#pragma GCC diagnostic pop
#endif