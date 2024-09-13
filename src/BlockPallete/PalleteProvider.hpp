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
class BlockStylerFactory;

struct BlockTemplate
{
	std::string template_name;
	std::string class_name;
	std::string styler_name;
	std::vector<model::BlockProperty> default_properties;
	model::BlockStyleProperties style_properties;
};

struct PalleteElement
{
	std::string block_template;
	model::BlockModel block{ model::BlockId{0}, model::Rect{0,0,0,0} };
	std::unique_ptr<BlockStyler> styler;
	std::shared_ptr<TextPainter> text_painter;
};

class PalleteProvider
{
public:
	PalleteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory);
	void AddElement(const BlockTemplate& temp);
	void AddElement(std::unique_ptr<PalleteElement> element) { m_elements.push_back(std::move(element)); }

	BlockStylerFactory& GetStylerFactory() { return *m_blockStyleFactory; }
	std::span<const std::unique_ptr<PalleteElement>> GetElements() { return m_elements; }
private:
	std::vector<std::unique_ptr<PalleteElement>> m_elements;
	std::shared_ptr<BlockClassesManager> m_classesManager;
	std::shared_ptr<BlockStylerFactory> m_blockStyleFactory;
};

}