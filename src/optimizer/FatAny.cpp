#include "FatAny.hpp"
#include "toolgui/NodeMacros.h"

opt::FatAny::FatAny(opt::FatAny&& other) noexcept
	: p_manager(other.p_manager)
{
	if (p_manager)
	{
		p_manager->move_construct(other.m_data.buff.data(), m_data.buff.data());
	}
}

opt::FatAny& opt::FatAny::operator=(opt::FatAny&& other) noexcept
{
	if (&other != this)
	{
		if (p_manager)
		{
			try {
				p_manager->destruct(m_data.buff.data());
			}
			catch (...)
			{ }
		}
		if (other.p_manager)
		{
			other.p_manager->move_construct(other.m_data.buff.data(), m_data.buff.data());
		}
		p_manager = other.p_manager;
	}

	return *this;
}

opt::FatAny::~FatAny()
{
	if (p_manager)
	{
		p_manager->destruct(m_data.buff.data());
	}
}
