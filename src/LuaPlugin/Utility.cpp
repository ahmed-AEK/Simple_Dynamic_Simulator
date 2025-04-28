#include "LuaPlugin/Utility.hpp"

tl::expected<std::string, std::string> node::lua::ReadLuaFile(std::string_view file_name, logging::Logger m_logger)
{
	std::string path_str{ file_name };
	auto* current_dir = SDL_GetCurrentDirectory();
	std::string full_path = std::string{ current_dir } + "LuaScripts/" + path_str;
	SDL_free(current_dir);
	SDL_IOStream* file_stream = SDL_IOFromFile(full_path.c_str(), "r");
	if (!file_stream)
	{
		return tl::unexpected<std::string>{std::format("Failed to open file: {} , reason: {}", path_str, SDL_GetError())};
	}
	std::string file_content;
	std::array<char, 1024> temp_buffer;
	while (auto bytes_read = SDL_ReadIO(file_stream, temp_buffer.data(), 1024))
	{
		if (file_content.size() > 1024 * 1024 * 8)
		{
			return tl::unexpected<std::string>{std::format("File too big!: {}", path_str)};
		}
		file_content.insert(file_content.end(), temp_buffer.data(), temp_buffer.data() + bytes_read);
	}
	SDL_CloseIO(file_stream);
	file_stream = nullptr;
	m_logger.LogDebug("Read Lua file bytes total = %d", static_cast<int>(file_content.size()));

	return file_content;
}