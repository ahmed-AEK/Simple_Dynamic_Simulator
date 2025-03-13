#pragma once

#include "PluginAPI/BlocksPlugin.hpp"

namespace node
{


class IPluginRuntime
{
public:
	using GetNameCallback = void(*)(void*, std::string_view);
	virtual void GetName(GetNameCallback cb, void* context) const = 0;
	virtual void GetPlugin(std::string_view path, IBlocksPlugin** plugin_ptr) = 0;
	virtual void GetDefaultPlugin(IBlocksPlugin** plugin_ptr) = 0;
	virtual void Destroy() { delete this; }
protected:
	virtual ~IPluginRuntime() = default;
};


namespace detail
{
	struct PluginRuntimeDeleter
	{
		void operator()(IPluginRuntime* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using PluginRuntimePtr = std::unique_ptr<IPluginRuntime, detail::PluginRuntimeDeleter>;

template <typename T, typename...Args>
PluginRuntimePtr make_PluginRuntime(Args&&...args)
{
	return PluginRuntimePtr{ new T{std::forward<Args>(args)...} };
}

}