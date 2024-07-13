#pragma once

#include <NodeModels/BlockModel.hpp>
#include <span>
#include <memory>
#include <vector>
#include <string>
#include "toolgui/NodeMacros.h"
#include "SDL_Framework/SDL_headers.h"

namespace node
{

	struct BlockStyler
	{
		static constexpr int SocketLength = 15;
		void PositionNodes(model::BlockModel& block)
		{
			{
				auto&& in_sockets = block.GetSockets(
					node::model::BlockSocketModel::SocketType::input);
				int in_spacing = static_cast<int>(block.GetBounds().height / (in_sockets.size() + 1));
				int in_counter = 1;
				for (auto&& sock : in_sockets)
				{
					sock.SetPosition({2, 
						in_spacing * in_counter - SocketLength/2});
					in_counter++;
				}
			}
			{
				auto&& output_sockets = block.GetSockets(
					node::model::BlockSocketModel::SocketType::output);
				int out_spacing = static_cast<int>(block.GetBounds().height /
					(output_sockets.size() + 1));
				int out_counter = 1;
				for (auto&& sock : output_sockets)
				{
					sock.SetPosition({ block.GetBounds().width - 2 - SocketLength, 
						out_spacing * out_counter - SocketLength/2});
					out_counter++;
				}
			}
		}

		void DrawBlock(SDL_Renderer* renderer, const model::BlockModel& block,
			const SDL_Rect& region)
		{
			UNUSED_PARAM(renderer);
			UNUSED_PARAM(block);
			UNUSED_PARAM(region);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &region);
			SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
			SDL_Rect inner_rect{ region.x + 2, region.y + 2, region.w - 4, region.h - 4 };
			SDL_RenderFillRect(renderer, &inner_rect);

			for (auto&& sock : 
				block.GetSockets(node::model::BlockSocketModel::SocketType::input))
			{
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_Rect draw_area = { region.x + sock.GetPosition().x, 
					region.y + sock.GetPosition().y,
				SocketLength, SocketLength };
				SDL_RenderFillRect(renderer, &draw_area);
			}
			for (auto&& sock : 
				block.GetSockets(node::model::BlockSocketModel::SocketType::output))
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
				SDL_Rect draw_area = { region.x + sock.GetPosition().x,
					region.y + sock.GetPosition().y,
				SocketLength, SocketLength };
				SDL_RenderFillRect(renderer, &draw_area);
			}
		}

	};

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