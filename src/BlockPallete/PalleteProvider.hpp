#pragma once

#include <NodeModels/BlockModel.hpp>
#include <span>
#include <memory>
#include <vector>
#include <string>
#include "toolgui/NodeMacros.h"
#include "SDL_Framework/SDL_headers.h"
#include "NodeSDLStylers/BlockStyler.hpp"

namespace node
{
	struct PalleteElement
	{
		std::string block_template;
		model::BlockModel block{ 0, model::Rect{0,0,0,0} };
		std::shared_ptr<BlockStyler> styler;
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