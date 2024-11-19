#pragma once

#include "BlockClasses/BlockClass.hpp"

namespace node
{

	class ComparatorBlockClass : public BlockClass
	{
	public:
		ComparatorBlockClass();

		const std::vector<model::BlockProperty>& GetClassProperties() override;
		std::vector<model::BlockSocketModel::SocketType>
			CalculateSockets(const std::vector<model::BlockProperty>& properties) override;
		const std::string_view& GetDescription() const override;
		bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) override;
		BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) override;
		GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) override;
	};

}