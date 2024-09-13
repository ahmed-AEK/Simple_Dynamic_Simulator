#include "BlockStylerFactory.hpp"

node::BlockStylerFactory::BlockStylerFactory()
{
}

void node::BlockStylerFactory::AddStyler(std::string name, styler_functor func)
{
	m_factoryFunctors.emplace(std::move(name), std::move(func));
}

std::unique_ptr<node::BlockStyler> node::BlockStylerFactory::GetStyler(std::string name, const model::BlockStyleProperties& style_prop)
{
	auto it = m_factoryFunctors.find(name);
	assert(it != m_factoryFunctors.end());
	if (it != m_factoryFunctors.end())
	{
		return it->second(style_prop);
	}
	return nullptr;
}
