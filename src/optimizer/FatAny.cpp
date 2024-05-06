#include "FatAny.hpp"
#include "toolgui/NodeMacros.h"
#include <stdexcept>


constexpr opt::AnyManager empty_manager =
{
	+[](void* ptr) { UNUSED_PARAM(ptr); },
	+[](void* src, void* dst) { UNUSED_PARAM(src); UNUSED_PARAM(dst); }
};

opt::FatAny::FatAny(opt::FatAny&& other) noexcept
	: m_manager(other.m_manager), m_contained(other.m_contained)
{
	m_manager.move_construct(other.m_data.data(), m_data.data());
}

opt::FatAny& opt::FatAny::operator=(opt::FatAny&& other) noexcept
{
	if (&other != this)
	{
		if (m_contained != std::type_index(typeid(nullptr)))
		{
			try {
				m_manager.destruct(m_data.data());
			}
			catch (...)
			{ }
		}
		other.m_manager.move_construct(other.m_data.data(), m_data.data());
		m_manager = other.m_manager;
		m_contained = other.m_contained;
	}

	return *this;
}

opt::FatAny::~FatAny()
{
	m_manager.destruct(m_data.data());
}
