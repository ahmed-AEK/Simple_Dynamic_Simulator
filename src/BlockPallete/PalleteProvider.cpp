#include "PalleteProvider.hpp"

#include "BlockClasses/BlockClass.hpp"
#include "BlockClasses/BlockClassesManager.hpp"

#include "BlockPallete.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"
#include <algorithm>
#include <iterator>

node::PalleteProvider::PalleteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory)
	:m_classesManager{ manager }, m_blockStyleFactory{std::move(style_factory)}
{
	assert(m_classesManager);
	assert(m_blockStyleFactory);
}

void node::PalleteProvider::AddElement(const BlockTemplate& temp)
{
	assert(m_classesManager);
	if (temp.data.GetFunctionalData())
	{
		AddFunctionalElemnt(temp);
	}
	else
	{
		SDL_Log("unknown block template !");
	}
}

void node::PalleteProvider::AddFunctionalElemnt(const BlockTemplate& temp)
{
	auto block_data_ptr = temp.data.GetFunctionalData();
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
	auto block = model::BlockModel{ model::BlockId{0}, model::BlockType::Functional, {0,0,BlockPallete::ElementWidth, BlockPallete::ElementHeight} };
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

	m_elements.push_back(
		std::make_unique<PalleteElement>(PalleteElement{
			temp.template_name,
			std::move(block),
			model::BlockData{*block_data_ptr},
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
			)
	);
}
