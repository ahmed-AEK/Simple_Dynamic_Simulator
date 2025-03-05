#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockData.hpp"

#include <any>
#include <variant>

namespace opt
{
	class DiffEquation;
	class NLEquation;
	class NLStatefulEquation;
	class Observer;
	class SourceEq;
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

using BlockFunctor = typename std::variant<opt::DiffEquation, opt::NLEquation, opt::NLStatefulEquation, opt::Observer, opt::SourceEq>;

struct BlockTemplate
{
	std::string category;
	std::string template_name;
	model::BlockData data;
	std::string styler_name;
	model::BlockStyleProperties style_properties;
};

class BlockClass
{
public:
	BlockClass(std::string name);
	virtual ~BlockClass();

	void SetName(std::string name) { m_name = name; }
	const std::string& GetName() const { return m_name; }
	virtual const std::string_view& GetDescription() const = 0;
	
	virtual const std::vector<model::BlockProperty>& GetDefaultClassProperties() = 0;
	virtual bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) = 0;

	virtual std::vector<model::BlockSocketModel::SocketType> 
		CalculateSockets(const std::vector<model::BlockProperty>& properties) = 0;
	virtual BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) = 0;

	using GetFunctorResult = std::variant<BlockFunctor, std::vector<BlockFunctor>>;
	virtual GetFunctorResult GetFunctor(const std::vector<model::BlockProperty>& properties) = 0;

	virtual std::unique_ptr<BlockDialog> CreateBlockDialog(Scene& scene, model::BlockModel& model, 
		model::FunctionalBlockData& data, std::any& simulation_data);
	virtual bool HasBlockDialog() const { return false; }
private:
	std::string m_name;
};

}
