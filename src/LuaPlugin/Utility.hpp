#pragma once

namespace node
{

namespace lua
{

tl::expected<std::string, std::string> ReadLuaFile(std::string_view file_name, logging::Logger m_logger = logger(logging::LogCategory::Extension));

}
}