#pragma once

#include "NodeModels/BlockModel.hpp"

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

enum class BlockType
{
	Source,
	Sink,
	Differential,
	Stateful,
	Stateless,
};

using BlockFunctor = typename std::variant<opt::DiffEquation, opt::NLEquation, opt::NLStatefulEquation, opt::Observer, opt::SourceEq>;

class BlockClass
{
public:
	BlockClass(std::string name);
	virtual ~BlockClass();

	void SetName(std::string name) { m_name = name; }
	const std::string& GetName() const { return m_name; }
	
	virtual const std::vector<model::BlockProperty>& GetClassProperties() = 0;
	virtual bool ValidateClassProperties(const std::vector<model::BlockProperty>& properties) = 0;

	virtual std::vector<model::BlockSocketModel::SocketType> 
		CalculateSockets(const std::vector<model::BlockProperty>& properties) = 0;
	virtual BlockType GetBlockType(const std::vector<model::BlockProperty>& properties) = 0;
	virtual BlockFunctor GetFunctor(const std::vector<model::BlockProperty>& properties) = 0;
private:
	std::string m_name;
};

}
