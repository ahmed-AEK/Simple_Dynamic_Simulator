#pragma once

#include <span>
#include <memory>
#include <vector>

namespace node
{
	struct PalleteElement
	{

	};

	class PalleteProvider
	{
	public:
		std::span<std::shared_ptr<PalleteElement>> GetElements() { return m_elements; }
		void AddElement(std::shared_ptr<PalleteElement> element) { m_elements.push_back(element); }
	private:
		std::vector<std::shared_ptr<PalleteElement>> m_elements;
	};
}