#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "toolgui/NodeMacros.h"
#include "SDL_Framework/SDL_headers.h"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeModels/BlockData.hpp"
#include "NodeModels/Observer.hpp"
#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"

#include <span>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <ranges>
#include "boost/container_hash/hash.hpp"

namespace node
{
class BlockClassesManager;
class BlockStylerFactory;

struct PaletteElement
{
	std::string block_template;
	model::BlockModel block{ model::BlockId{0}, model::BlockType::Functional, model::Rect{0,0,0,0} };
	model::BlockData data;
	std::unique_ptr<BlockStyler> styler;
	std::shared_ptr<TextPainter> text_painter;
};

struct BlockPaletteChange
{
	struct CategoryAdded
	{
		std::string_view category;
	};

	struct ElementAdded
	{
		std::string_view category;
		PaletteElement& elem;
	};

	using Event = std::variant<CategoryAdded, ElementAdded>;
	Event e;
};

class PaletteProvider : public MultiPublisher<BlockPaletteChange>
{
public:
	enum class CategoryId : uint32_t {};
	enum class ElementId : uint32_t {};
	struct ElementUniqueId
	{
		CategoryId cat_id;
		ElementId temp_id;
		auto operator<=>(const ElementUniqueId&) const = default;
	};

	struct CategoryMapping
	{
		CategoryId id;
		std::string name;
	};
	struct ElementMapping
	{
		ElementId id;
		PaletteElement element;
	};

	PaletteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory);
	std::optional<ElementUniqueId> AddElement(const BlockTemplate& temp);

	BlockStylerFactory& GetStylerFactory() { return *m_blockStyleFactory; }
	std::span<const ElementMapping> GetCategoryElementsMap(const std::string& category) const;
	auto GetCategoryElements(const std::string& category) const
	{
		auto s = GetCategoryElementsMap(category);
		return s | std::ranges::views::transform([](const ElementMapping& m) ->const PaletteElement& { return m.element; });
	}
	std::span<const CategoryMapping> GetCategories() const;
private:
	std::optional<ElementUniqueId> AddFunctionalElemnt(const BlockTemplate& temp);
	std::optional<ElementUniqueId> AddSubsystemElement(const BlockTemplate& temp);
	std::optional<ElementUniqueId> AddPortElement(const BlockTemplate& temp);

	using palette_storage = std::unordered_map<CategoryId, std::vector<ElementMapping>>;
	palette_storage::iterator EnsureCategory(const std::string& str);

	palette_storage m_elements;

	uint32_t m_next_cat_id{};
	uint32_t m_next_elem_id{};

	std::vector<CategoryMapping> m_categories;
	std::shared_ptr<BlockClassesManager> m_classesManager;
	std::shared_ptr<BlockStylerFactory> m_blockStyleFactory;
	logging::Logger m_logger = logger(logging::LogCategory::Core);
};

}

template <>
struct std::hash<node::PaletteProvider::ElementUniqueId>
{
	std::size_t operator()(const node::PaletteProvider::ElementUniqueId& k) const
	{
		size_t hash = 0;
		boost::hash_combine(hash, static_cast<size_t>(k.cat_id));
		boost::hash_combine(hash, static_cast<size_t>(k.temp_id));
		return hash;
	}
};