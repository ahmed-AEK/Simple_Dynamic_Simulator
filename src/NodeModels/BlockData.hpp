#pragma once

#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/SubsystemBlocksDataManager.hpp"
#include "NodeModels/PortBlocksDataManager.hpp"
#include "NodeModels/BlockModel.hpp"
#include <variant>

namespace node
{
	
namespace model
{
	struct BlockData
	{
		FunctionalBlockData* GetFunctionalData()
		{
			return std::get_if<FunctionalBlockData>(&data);
		}
		const FunctionalBlockData* GetFunctionalData() const
		{
			return std::get_if<FunctionalBlockData>(&data);
		}

		SubsystemBlockData* GetSubsystemData()
		{
			return std::get_if<SubsystemBlockData>(&data);
		}
		const SubsystemBlockData* GetSubsystemData() const
		{
			return std::get_if<SubsystemBlockData>(&data);
		}

		PortBlockData* GetPortData()
		{
			return std::get_if<PortBlockData>(&data);
		}
		const PortBlockData* GetPortData() const
		{
			return std::get_if<PortBlockData>(&data);
		}

		std::variant<std::monostate, FunctionalBlockData, SubsystemBlockData, PortBlockData> data;
	};

	struct BlockDataCRef
	{
		std::reference_wrapper<const BlockModel> block;

		const FunctionalBlockData* GetFunctionalData() const 
		{

			return &std::get_if<std::reference_wrapper<const FunctionalBlockData>>(&data)->get();
		}

		const SubsystemBlockData* GetSubsystemData() const 
		{

			return &std::get_if<std::reference_wrapper<const SubsystemBlockData>>(&data)->get();
		}

		const PortBlockData* GetPortData() const
		{
			return &std::get_if<std::reference_wrapper<const PortBlockData>>(&data)->get();
		}

		using FunctionalRef = std::reference_wrapper<const FunctionalBlockData>;
		using SubsytemRef = std::reference_wrapper<const SubsystemBlockData>;
		using PortRef = std::reference_wrapper<const PortBlockData>;

		std::variant<std::monostate, 
			FunctionalRef,
			SubsytemRef,
			PortRef> data;
	};
}

}