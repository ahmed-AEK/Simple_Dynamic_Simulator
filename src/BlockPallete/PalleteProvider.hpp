#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "toolgui/NodeMacros.h"
#include "SDL_Framework/SDL_headers.h"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeModels/BlockData.hpp"
#include "NodeModels/Observer.hpp"

#include <span>
#include <memory>
#include <vector>
#include <string>

namespace node
{
class BlockClassesManager;
class BlockStylerFactory;

struct BlockTemplate
{
	std::string category;
	std::string template_name;
	model::BlockData data;
	std::string styler_name;
	model::BlockStyleProperties style_properties;
};

struct PalleteElement
{
	std::string block_template;
	model::BlockModel block{ model::BlockId{0}, model::BlockType::Functional, model::Rect{0,0,0,0} };
	model::BlockData data;
	std::unique_ptr<BlockStyler> styler;
	std::shared_ptr<TextPainter> text_painter;
};

struct BlockPalleteChange
{
	struct CategoryAdded
	{
		std::string_view category;
	};

	CategoryAdded e;
};

class PalleteProvider : public SinglePublisher<BlockPalleteChange>
{
public:
	PalleteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory);
	void AddElement(const BlockTemplate& temp);
	void AddElement(const std::string& category, std::unique_ptr<PalleteElement> element);

	BlockStylerFactory& GetStylerFactory() { return *m_blockStyleFactory; }
	const std::vector<std::unique_ptr<PalleteElement>>* GetCategoryElements(const std::string& category) const;
	std::vector<std::string_view> GetCategories() const;
private:
	void AddFunctionalElemnt(const BlockTemplate& temp);
	void AddSubsystemElement(const BlockTemplate& temp);
	void AddPortElement(const BlockTemplate& temp);

	std::unordered_map<std::string, std::vector<std::unique_ptr<PalleteElement>>> m_elements;
	std::shared_ptr<BlockClassesManager> m_classesManager;
	std::shared_ptr<BlockStylerFactory> m_blockStyleFactory;
};

}