#include "PalleteProvider.hpp"

#include "BlockClasses/BlockClass.hpp"
#include "BlockClasses/BlockClassesManager.hpp"

#include "BlockPallete.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

node::PalleteProvider::PalleteProvider(std::shared_ptr<BlockClassesManager> manager, std::shared_ptr<BlockStylerFactory> style_factory)
	:m_classesManager{ manager }, m_blockStyleFactory{std::move(style_factory)}
{
	assert(m_classesManager);
	assert(m_blockStyleFactory);
}

void node::PalleteProvider::AddElement(const BlockTemplate& temp)
{
	assert(m_classesManager);
	auto block_class = m_classesManager->GetBlockClassByName(temp.class_name);
	assert(block_class);
	if (!block_class)
	{
		return;
	}

	assert(block_class->ValidateClassProperties(temp.default_properties));
	if (!block_class->ValidateClassProperties(temp.default_properties))
	{
		return;
	}

	auto sockets_types = block_class->CalculateSockets(temp.default_properties);
	auto block = model::BlockModel{ model::BlockId{0}, {0,0,BlockPallete::ElementWidth, BlockPallete::ElementHeight} };
	model::id_int socket_id = 0;
	for (const auto& sock_type : sockets_types)
	{
		block.AddSocket(model::BlockSocketModel{ sock_type, model::SocketId{socket_id} });
		socket_id++;
	}

	block.SetClass(temp.class_name);
	block.SetStyler(temp.styler_name);
	block.SetStylerProperties(temp.style_properties);
	auto styler = m_blockStyleFactory->GetStyler(temp.styler_name, temp.style_properties);

	assert(styler);
	styler->PositionNodes(block);
	auto& properties = block.GetProperties();
	for (const auto& prop : temp.default_properties)
	{
		properties.push_back(prop);
	}

	m_elements.push_back(
		std::make_unique<PalleteElement>(PalleteElement{
			temp.template_name,
			std::move(block),
			std::move(styler),
			std::make_shared<TextPainter>(nullptr),
			}
			)
	);
}
