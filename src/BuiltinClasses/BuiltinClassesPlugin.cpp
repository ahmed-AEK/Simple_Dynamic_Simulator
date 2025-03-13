#include "BuiltinClassesPlugin.hpp"
#include "PluginAPI/BlockClassHelpers.hpp"

#include "GainBlockClass.hpp"
#include "ConstantSourceClass.hpp"
#include "ScopeDisplayClass.hpp"
#include "RampSourceClass.hpp"
#include "IntegrationBlockClass.hpp"
#include "DerivativeBlockClass.hpp"
#include "AddSimpleClass.hpp"
#include "MultiplyBlockClass.hpp"
#include "SineSourceClass.hpp"
#include "StepSourceClass.hpp"
#include "ComparatorClass.hpp"

#include "NodeSDLStylers/DefaultBlockStyler.hpp"
#include "NodeSDLStylers/TextBlockStyler.hpp"
#include "NodeSDLStylers/GainBlockStyler.hpp"
#include "NodeSDLStylers/PropertyPrintStyler.hpp"
#include "NodeSDLStylers/SVGBlockStyler.hpp"

namespace 
{

namespace detail
{

static std::span<const std::shared_ptr<node::BlockClass>> get_builtin_classes()
{
    using namespace node;
    static const std::shared_ptr<node::BlockClass> classes[] =
    {
        std::make_shared<GainBlockClass>(),
        std::make_shared<ConstantSourceClass>(),
        std::make_shared<ScopeDisplayClass>(),
        std::make_shared<RampSourceClass>(),
        std::make_shared<IntegrationBlockClass>(),
        std::make_shared<DerivativeBlockClass>(),
        std::make_shared<AddSimpleBlockClass>(),
        std::make_shared<MultiplyBlockClass>(),
        std::make_shared<SineSourceClass>(),
        std::make_shared<StepSourceClass>(),
        std::make_shared<ComparatorBlockClass>()
    };
    return classes;
}

static std::span<const node::BlockTemplate> get_builtin_blocks()
{
    using namespace node;
    static const node::BlockTemplate blocks[] =
    {
        
    {
        "Subsystem",
        "Subsystem",
        model::SubsystemBlockData{
            "Local",
            SubSceneId{0}
        },
        "Default",
        model::BlockStyleProperties{}
    },
    {
        "Subsystem",
        "Input",
        model::PortBlockData{
            model::SocketId{0},
            model::SocketType::input
        },
        "Default",
        model::BlockStyleProperties{}
    },
    {
        "Subsystem",
        "Output",
        model::PortBlockData{
            model::SocketId{0},
            model::SocketType::output
        },
        "Default",
        model::BlockStyleProperties{}
    },
    {
        "Math",
        "Gain",
        model::FunctionalBlockData{
            "Gain",
            std::vector<model::BlockProperty>{
                *model::BlockProperty::Create("Multiplier", model::BlockPropertyType::FloatNumber, 1.0)
            }
        },
        "Gain",
        model::BlockStyleProperties{}
    },
    {
        "Math",
        "Add",
        model::FunctionalBlockData{
            "Add Simple", {}
        },
        "Text",
        model::BlockStyleProperties{{{TextBlockStyler::key_text, "+"}}}
    },
    {
        "Math",
        "Integration",
        model::FunctionalBlockData{
            "Integration", {}
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/integral.svg"}}}
    },
    {
        "Math",
        "Derivative",
        model::FunctionalBlockData{
            "Derivative", {}
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/derivative.svg"}}}
    },
    {
        "Sources",
        "Constant Source",
        model::FunctionalBlockData{
            "Constant Source",
            std::vector<model::BlockProperty>{
                *model::BlockProperty::Create("Value", model::BlockPropertyType::FloatNumber, 1.0)
            }
        },
        "Property Printer",
        model::BlockStyleProperties{{{PropertyPrintStyler::printed_key_text, "Value"}}}
    },
    {
        "Sources",
        "Ramp",
        model::FunctionalBlockData{
            "Ramp",
            std::vector<model::BlockProperty>{
                *model::BlockProperty::Create("Slope", model::BlockPropertyType::FloatNumber, 1.0)
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/ramp.svg"}}}
    },
    {
        "Sinks",
        "Scope",
        model::FunctionalBlockData {
            "Scope Display",
            std::vector<model::BlockProperty>{
                *model::BlockProperty::Create("Inputs", model::BlockPropertyType::UnsignedInteger, static_cast<uint64_t>(1))
            },
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/scope.svg"}}}
    },
    {
        "Math",
        "Multiply",
        model::FunctionalBlockData{
            "Multiply", {}
        },
        "Text",
        model::BlockStyleProperties{{{TextBlockStyler::key_text, "X"}}}
    },
    {
        "Sources",
        "Sine",
        model::FunctionalBlockData{
            "Sine",
            std::vector<model::BlockProperty>{
                *model::BlockProperty::Create("Phase_deg", model::BlockPropertyType::FloatNumber, 0.0),
                *model::BlockProperty::Create("Freq_hz", model::BlockPropertyType::FloatNumber, 1.0),
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/sine.svg"}}}
    },
    {
        "Sources",
        "Step",
        model::FunctionalBlockData{
            "Step",
            std::vector<model::BlockProperty>{
                *node::model::BlockProperty::Create("Initial Value", node::model::BlockPropertyType::FloatNumber, 0.0 ),
                *node::model::BlockProperty::Create("Final Value", node::model::BlockPropertyType::FloatNumber, 1.0 ),
                *node::model::BlockProperty::Create("Step Time", node::model::BlockPropertyType::FloatNumber, 1.0 ),
                *node::model::BlockProperty::Create("Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 ),
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/step.svg"}}}
    },
    {
        "Math",
        "Comparator",
        model::FunctionalBlockData{
            "Comparator",
            std::vector<model::BlockProperty>{
                *node::model::BlockProperty::Create("threshold", node::model::BlockPropertyType::FloatNumber, 0.0 ),
                *node::model::BlockProperty::Create("Rise Time", node::model::BlockPropertyType::FloatNumber, 1e-6 ),
            }
        },
        "SVG Styler",
        model::BlockStyleProperties{{{SVGBlockStyler::SVG_PATH_PROPERTY_STRING, "assets/comparator.svg"}}}
    }
    };
    return blocks;
}
}
}

node::BuiltinClassesPlugin::BuiltinClassesPlugin()
    :IBlocksPlugin{}
{
}

void node::BuiltinClassesPlugin::GetPluginName(GetPluginNameCallback cb, void* context)
{
    constexpr std::string_view name = "Builtin";
    cb(context, name);
}

std::vector<std::shared_ptr<node::BlockClass>> node::BuiltinClassesPlugin::GetClasses()
{
    auto classes = ::detail::get_builtin_classes();
    return {classes.begin(), classes.end()};
}

void node::BuiltinClassesPlugin::GetBlocks(GetBlocksCallback cb, void* context)
{
    auto blocks = ::detail::get_builtin_blocks();
    auto group = helper::BlockTemplateGroupToC(blocks);

    cb(context, group.block_templates);
}

void node::NativePluginsRuntime::GetName(GetNameCallback cb, void* context) const
{
    constexpr std::string_view name = "NativeCpp";
    cb(context, name);
}

void node::NativePluginsRuntime::GetPlugin(std::string_view path, node::IBlocksPlugin** plugin_ptr)
{
    UNUSED_PARAM(path);
    UNUSED_PARAM(plugin_ptr);
}

void node::NativePluginsRuntime::GetDefaultPlugin(node::IBlocksPlugin** plugin_ptr)
{
    struct StaticBuiltinPlugin: public BuiltinClassesPlugin
    {
        using BuiltinClassesPlugin::BuiltinClassesPlugin;
        void Destroy() override {}
    };

    static StaticBuiltinPlugin plugin;
    *plugin_ptr = &plugin;
}