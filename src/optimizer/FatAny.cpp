#include "FatAny.hpp"
#include "toolgui/NodeMacros.h"


constexpr opt::AnyManager empty_manager =
{
	+[](void* ptr) { UNUSED_PARAM(ptr); },
	+[](void* src, void* dst) { UNUSED_PARAM(src); UNUSED_PARAM(dst); }
};

opt::FatAny::FatAny(opt::FatAny&& other) noexcept
	: m_manager(other.m_manager), p_manager(other.p_manager)
{
	m_manager.move_construct(other.m_data.buff.data(), m_data.buff.data());
}

opt::FatAny& opt::FatAny::operator=(opt::FatAny&& other) noexcept
{
	if (&other != this)
	{
		if (p_manager != nullptr)
		{
			try {
				m_manager.destruct(m_data.buff.data());
			}
			catch (...)
			{ }
		}
		other.m_manager.move_construct(other.m_data.buff.data(), m_data.buff.data());
		m_manager = other.m_manager;
		p_manager = other.p_manager;
	}

	return *this;
}

opt::FatAny::~FatAny()
{
	m_manager.destruct(m_data.buff.data());
}
