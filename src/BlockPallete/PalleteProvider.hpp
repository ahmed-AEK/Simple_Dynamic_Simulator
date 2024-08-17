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
class BlockClassesManager;

struct BlockTemplate
{
	std::string template_name;
	std::string class_name;
	std::string styler_name;
	std::vector<model::BlockProperty> default_properties;
};

struct PalleteElement
{
	std::string block_template;
	model::BlockModel block{ model::BlockId{0}, model::Rect{0,0,0,0} };
	std::shared_ptr<BlockStyler> styler;
};

class PalleteProvider
{
public:
	PalleteProvider(std::shared_ptr<BlockClassesManager> manager);
	void AddElement(const BlockTemplate& temp);
	void AddElement(std::unique_ptr<PalleteElement> element) { m_elements.push_back(std::move(element)); }

	std::span<const std::unique_ptr<PalleteElement>> GetElements() { return m_elements; }
private:
	std::vector<std::unique_ptr<PalleteElement>> m_elements;
	std::shared_ptr<BlockClassesManager> m_classesManager;
};

}