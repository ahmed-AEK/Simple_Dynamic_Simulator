#include "PaletteProvider.hpp"
#include "PaletteBlocksViewer.hpp"


node::PaletteProvider::PaletteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory)
	:m_classesManager{ manager }, m_blockStyleFactory{std::move(style_factory)}
{
	assert(m_classesManager);
	assert(m_blockStyleFactory);
}

void node::PaletteProvider::AddElement(const BlockTemplate& temp)
{
	assert(m_classesManager);
	assert(temp.category.size());
	if (temp.data.GetFunctionalData())
	{
		AddFunctionalElemnt(temp);
	}
	else if (temp.data.GetSubsystemData())
	{
		AddSubsystemElement(temp);
	}
	else if (temp.data.GetPortData())
	{
		AddPortElement(temp);
	}
	else
	{
		SDL_Log("unknown block template !");
	}
}

void node::PaletteProvider::AddElement(const std::string& category, std::unique_ptr<PaletteElement> element)
{
	m_elements[category].push_back(std::move(element));
}


const std::vector<std::unique_ptr<node::PaletteElement>>* node::PaletteProvider::GetCategoryElements(const std::string& category) const
{
	auto it = m_elements.find(category);
	if (it != m_elements.end())
	{
		return &(it->second);
	}
	return nullptr;
}

std::vector<std::string_view> node::PaletteProvider::GetCategories() const
{
	auto result = std::vector<std::string_view>();
	for (auto&& [category, vec] : m_elements)
	{
		result.push_back(category);
	}
	return result;
}

void node::PaletteProvider::AddFunctionalElemnt(const BlockTemplate& temp)
{
	auto* block_data_ptr = temp.data.GetFunctionalData();
	assert(block_data_ptr);
	auto block_class = m_classesManager->GetBlockClassByName(block_data_ptr->block_class);
	assert(block_class);
	if (!block_class)
	{
		return;
	}

	assert(block_class->ValidateClassProperties(block_data_ptr->properties));
	if (!block_class->ValidateClassProperties(block_data_ptr->properties))
	{
		return;
	}

	auto sockets_types = block_class->CalculateSockets(block_data_ptr->properties);
	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::Functional, {0,0,PaletteBlocksViewer::ElementWidth, PaletteBlocksViewer::ElementHeight} };
	model::id_int socket_id = 0;
	for (const auto& sock_type : sockets_types)
	{
		block.AddSocket(model::BlockSocketModel{ sock_type, model::SocketId{socket_id} });
		socket_id++;
	}

	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);

	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, model::BlockDataCRef{ block, model::BlockDataCRef::FunctionalRef{*block_data_ptr} });

	assert(styler);
	styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());

	m_elements[temp.category].push_back(
		std::make_unique<PaletteElement>(PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
			)
	);
}

void node::PaletteProvider::AddSubsystemElement(const BlockTemplate& temp)
{
	auto* block_data_ptr = temp.data.GetSubsystemData();
	assert(block_data_ptr);

	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::SubSystem, {0,0,PaletteBlocksViewer::ElementWidth, PaletteBlocksViewer::ElementHeight} };

	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);

	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, model::BlockDataCRef{ block, model::BlockDataCRef::SubsytemRef{*block_data_ptr} });

	assert(styler);
	styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());

	m_elements[temp.category].push_back(
		std::make_unique<PaletteElement>(PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
			)
	);
}

void node::PaletteProvider::AddPortElement(const BlockTemplate& temp)
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

	m_elements[temp.category].push_back(
		std::make_unique<PaletteElement>(PaletteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
			)
	);
}
