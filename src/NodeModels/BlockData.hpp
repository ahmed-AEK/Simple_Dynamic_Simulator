#pragma once

#include "NodeModels/FunctionalBlocksManager.hpp"
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
		std::variant<std::monostate, FunctionalBlockData> data;
	};

	struct BlockDataCRef
	{
		std::reference_wrapper<const BlockModel> block;
		const FunctionalBlockData* GetFunctionalData() const {

			return &std::get_if<std::reference_wrapper<const FunctionalBlockData>>(&data)->get();
		}

		using FunctionalRef = std::reference_wrapper<const FunctionalBlockData>;

		std::variant<std::monostate, std::reference_wrapper<const FunctionalBlockData>> data;
	};
}

}