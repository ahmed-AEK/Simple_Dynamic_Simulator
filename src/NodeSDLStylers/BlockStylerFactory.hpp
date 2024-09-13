#pragma once

#include "NodeSDLStylers/BlockStyler.hpp"

#include <string>
#include <functional>
#include <memory>

namespace node
{

class BlockStylerFactory
{
public:
	using styler_functor = std::function<std::unique_ptr<BlockStyler>(const model::BlockStyleProperties&)>;
	BlockStylerFactory();
	void AddStyler(std::string name, styler_functor func);
	std::unique_ptr<BlockStyler> GetStyler(std::string name, const  model::BlockStyleProperties& style_prop);
private:
	std::unordered_map<std::string, styler_functor> m_factoryFunctors;
};
}