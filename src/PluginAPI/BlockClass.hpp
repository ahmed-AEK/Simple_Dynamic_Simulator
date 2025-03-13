#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockData.hpp"

#include <any>
#include <variant>

namespace opt
{
	struct DiffEquationWrapper;
	struct NLEquationWrapper;
	struct NLStatefulEquationWrapper;
	struct ObserverWrapper;
	struct SourceEqWrapper;
}

namespace node
{

class BlockDialog;
class Scene;

enum class BlockType
{
	Source,
	Sink,
	Differential,
	Stateful,
	Stateless,
};

using BlockFunctor = typename std::variant<opt::DiffEquationWrapper, opt::NLEquationWrapper, 
	opt::NLStatefulEquationWrapper, opt::ObserverWrapper, opt::SourceEqWrapper>;

struct BlockTemplate
{
	std::string category;
	std::string template_name;
	model::BlockData data;
	std::string styler_name;
	model::BlockStyleProperties style_properties;
};

class IBlockClass
{
public:
	virtual ~IBlockClass() = default;

	virtual std::string_view GetName() const = 0;
	virtual std::string_view GetDescription() const = 0;

	virtual std::span<const model::BlockProperty> GetDefaultClassProperties() const = 0;
	virtual bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) const = 0;

	virtual std::vector<model::BlockSocketModel::SocketType>
		CalculateSockets(const std::vector<model::BlockProperty>& properties) const = 0;
	virtual BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) const = 0;

	using GetFunctorResult = std::variant<BlockFunctor, std::vector<BlockFunctor>>;
	virtual GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) const = 0;

	virtual bool HasBlockDialog() const { return false; }
};

class BlockClass: public IBlockClass
{
public:
	BlockClass(std::string name);
	virtual ~BlockClass();

	void SetName(std::string name) { m_name = name; }
	std::string_view GetName() const override { return m_name; }

	virtual std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, model::BlockModel& model,
		model::FunctionalBlockData& data, std::any& simulation_data);
private:
	std::string m_name;
};

}
