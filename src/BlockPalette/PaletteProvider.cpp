#include "PaletteProvider.hpp"
#include "PaletteBlocksViewer.hpp"


static auto CalculateBlockSockets(std::span<const node::model::BlockProperty> properties, node::IBlockClass& block)
{
	node::CalculateSocketCallback cb;
	block.CalculateSockets(properties, cb);
	return std::move(cb.added_sockets);
}

node::PaletteProvider::PaletteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory)
	:m_classesManager{ manager }, m_blockStyleFactory{std::move(style_factory)}
{
	assert(m_classesManager);
	assert(m_blockStyleFactory);
}

std::optional<node::PaletteProvider::ElementUniqueId> node::PaletteProvider::AddElement(const BlockTemplate& temp)
{
	assert(m_classesManager);
	assert(temp.category.size());
	if (temp.data.GetFunctionalData())
	{
		return AddFunctionalElemnt(temp);
	}
	else if (temp.data.GetSubsystemData())
	{
		return AddSubsystemElement(temp);
	}
	else if (temp.data.GetPortData())
	{
		return AddPortElement(temp);
	}
	else
	{
		m_logger.LogError("unknown block template !");
		
		return std::nullopt;
	}
}

std::span<const node::PaletteProvider::ElementMapping> node::PaletteProvider::GetCategoryElementsMap(const std::string& category) const
{
	auto cat_it = std::find_if(m_categories.begin(), m_categories.end(), [&](const CategoryMapping& e) { return category == e.name; });
	if (cat_it == m_categories.end())
	{
		return {};
	}

	auto it = m_elements.find(cat_it->id);
	if (it != m_elements.end())
	{
		return it->second;
	}
	return {};
}

std::span<const node::PaletteProvider::CategoryMapping> node::PaletteProvider::GetCategories() const
{
	return m_categories;
}

std::optional<node::PaletteProvider::ElementUniqueId> node::PaletteProvider::AddFunctionalElemnt(const BlockTemplate& temp)
{
	auto* block_data_ptr = temp.data.GetFunctionalData();
	assert(block_data_ptr);
	auto block_class = m_classesManager->GetBlockClassByName(block_data_ptr->block_class);
	assert(block_class);
	if (!block_class)
	{
		return std::nullopt;
	}

	LightValidatePropertiesNotifier notifier;
	if (!block_class->ValidateClassProperties(block_data_ptr->properties, notifier) || notifier.errored)
	{
		m_logger.LogError("Validation of block '{}' in category '{}' failed", temp.template_name, temp.category);
	}

	auto sockets = CalculateBlockSockets(block_data_ptr->properties, *block_class);
	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::Functional, {0,0,PaletteBlocksViewer::ElementWidth, PaletteBlocksViewer::ElementHeight} };
	model::id_int socket_id = 0;
	for (const auto& socket : sockets)
	{
		block.AddSocket(model::BlockSocketModel{ socket.socket_type, model::SocketId{socket_id}, {}, {}, {}, socket.category });
		socket_id++;
	}

	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);

	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, model::BlockDataCRef{ block, model::BlockDataCRef::FunctionalRef{*block_data_ptr} });

	assert(styler);
	styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());

	auto it = EnsureCategory(temp.category);

	auto elem_id = ElementId{ m_next_elem_id };
	it->second.push_back(
		{ elem_id, PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
		}
	);
	m_next_elem_id++;
	Notify(BlockPaletteChange{ BlockPaletteChange::ElementAdded{temp.category, it->second.back().element} });
	return ElementUniqueId{ it->first ,elem_id };
}

std::optional<node::PaletteProvider::ElementUniqueId> node::PaletteProvider::AddSubsystemElement(const BlockTemplate& temp)
{
	auto* block_data_ptr = temp.data.GetSubsystemData();
	assert(block_data_ptr);

	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::SubSystem, {0,0,PaletteBlocksViewer::ElementWidth, PaletteBlocksViewer::ElementHeight} };

	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);

	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, model::BlockDataCRef{ block, model::BlockDataCRef::SubsytemRef{*block_data_ptr} });

	assert(styler);
	styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());

	auto it = EnsureCategory(temp.category);

	auto elem_id = ElementId{ m_next_elem_id };
	it->second.push_back(
		{ elem_id, PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
		}
	);
	m_next_elem_id++;
	Notify(BlockPaletteChange{ BlockPaletteChange::ElementAdded{temp.category, it->second.back().element} });
	return ElementUniqueId{ it->first, elem_id };
}

std::optional<node::PaletteProvider::ElementUniqueId> node::PaletteProvider::AddPortElement(const BlockTemplate& temp)
{
	auto* block_data_ptr = temp.data.GetPortData();
	assert(block_data_ptr);

	auto sockets_types = std::vector<model::SocketType>{};
	sockets_types.push_back(
		block_data_ptr->port_type == model::SocketType::input ? 
		model::SocketType::output : model::SocketType::input);
	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::Port, {0,0,PaletteBlocksViewer::ElementWidth, PaletteBlocksViewer::ElementHeight} };
	model::id_int socket_id = 0;
	for (const auto& sock_type : sockets_types)
	{
		block.AddSocket(model::BlockSocketModel{ sock_type, model::SocketId{socket_id} });
		socket_id++;
	}

	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);

	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, model::BlockDataCRef{ block, model::BlockDataCRef::PortRef{*block_data_ptr} });

	assert(styler);
	styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());

	auto it = EnsureCategory(temp.category);

	auto elem_id = ElementId{ m_next_elem_id };
	it->second.push_back(
		{ elem_id, PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
		}
	);
	m_next_elem_id++;
	Notify(BlockPaletteChange{ BlockPaletteChange::ElementAdded{temp.category, it->second.back().element} });
	return ElementUniqueId{ it->first ,elem_id };
}

node::PaletteProvider::palette_storage::iterator node::PaletteProvider::EnsureCategory(const std::string& str)
{
	auto it = std::find_if(m_categories.begin(), m_categories.end(), [&](const CategoryMapping& e) { return str == e.name; });
	if (it == m_categories.end())
	{
		auto cat_id = CategoryId{ m_next_cat_id };
		m_categories.push_back(CategoryMapping{ cat_id, str });
		m_next_cat_id++;
		auto res = m_elements.emplace(cat_id, palette_storage::mapped_type{});
		Notify(BlockPaletteChange{ BlockPaletteChange::CategoryAdded{ str } });
		return res.first;
	}
	else
	{
		auto it2 = m_elements.find(it->id);
		assert(it2 != m_elements.end());
		return it2;
	}
}
