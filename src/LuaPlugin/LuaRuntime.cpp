#include "LuaRuntime.hpp"
#include "LuaPlugin/LuaExpressionClass.hpp"
#include "LuaPlugin/LuaStatefulEqnClass.hpp"

#include "NodeSDLStylers/SVGBlockStyler.hpp"

namespace
{
    using namespace node;

    static std::span<node::IBlockClass*> get_lua_classes()
    {
        using namespace node;
        static const node::BlockClassPtr classes[] =
        {
            make_BlockClass<LuaExpressionClass>(),
            make_BlockClass<LuaStatefulEqnClass>(),
        };

        static auto classes_raw = [&]()
            {
                std::array<node::IBlockClass*, std::size(classes)> temp{};
                for (size_t i = 0; i < std::size(classes); i++)
                {
                    temp[i] = classes[i];
                }
                return temp;
            }();
        return classes_raw;
    }

    static std::span<const node::BlockTemplate> get_lua_blocks()
    {
        using namespace node;
        static const node::BlockTemplate blocks[] =
        {
        {
            "Lua",
            "Expression",
            model::FunctionalBlockData{
                "LuaExpression",
                {
                *node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 0),
                * node::model::BlockProperty::Create("Expression", node::model::BlockPropertyType::String, "t")
                },
            },
            "SVG Styler",
            model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/lua_logo.svg"}}}
        },

        {
            "Lua",
            "Stateful Equation",
            model::FunctionalBlockData{
                "LuaStatefulEqn",
                {
                *node::model::BlockProperty::Create("Inputs Count", node::model::BlockPropertyType::UnsignedInteger, 1),
                *node::model::BlockProperty::Create("Path", node::model::BlockPropertyType::String, "DerivativeBlock.lua"),
                },
            },
            "SVG Styler",
            model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/lua_logo.svg"}}}
        },
        };
        return blocks;
    }
}
void node::LuaRuntime::GetName(GetNameCallback cb, void* context) const
{
	constexpr std::string_view name = "Lua";
	cb(context, name);
}

void node::LuaRuntime::GetPlugin(std::string_view path, IBlocksPlugin** plugin_ptr)
{
	UNUSED_PARAM(path);
	UNUSED_PARAM(plugin_ptr);
}

void node::LuaRuntime::GetDefaultPlugin(IBlocksPlugin** plugin_ptr)
{
	struct StaticBuiltinPlugin : public LuaBasePlugin
	{
		using LuaBasePlugin::LuaBasePlugin;
		void Destroy() override {}
	};

	static StaticBuiltinPlugin plugin;
	*plugin_ptr = &plugin;
}

void node::LuaBasePlugin::GetPluginName(GetPluginNameCallback cb, void* context)
{
	constexpr std::string_view name = "LuaBase";
	cb(context, name);
}

void node::LuaBasePlugin::GetClasses(GetClassesCallback cb, void* context)
{
    auto classes = get_lua_classes();
    cb(context, classes);
}

void node::LuaBasePlugin::GetBlocks(GetBlocksCallback cb, void* context)
{
    auto blocks = get_lua_blocks();
    auto group = helper::BlockTemplateGroupToC(blocks);

    cb(context, group.block_templates);
}
