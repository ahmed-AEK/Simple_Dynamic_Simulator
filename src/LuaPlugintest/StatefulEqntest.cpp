//#pragma warning( push , 0)
#include "gtest/gtest.h"
#include "gtest/gtest-matchers.h"
//#pragma warning( pop ) 
#include "LuaPlugin/LuaStatefulEqn.hpp"

using namespace node::lua;

TEST(testStatefulEqn, testCreate)
{
    std::string code = R"(
    function apply(input, output, t, data)
    end

    function update(input, t, data)
    end
)";
    node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
    NLStatefulEqnBuilder builder{ log };
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    auto funcs = builder.build_lua_functions(code, lua);
    EXPECT_TRUE(funcs);
    EXPECT_FALSE(funcs->setup);
}

TEST(testStatefulEqn, testCreateSetup)
{
    std::string code = R"(
    function setup(data)
    end

    function apply(input, output, t, data)
    end

    function update(input, t, data)
    end
)";
    node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
    NLStatefulEqnBuilder builder{ log };
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    auto funcs = builder.build_lua_functions(code, lua);
    EXPECT_TRUE(funcs);
    EXPECT_TRUE(funcs->setup);
}


TEST(testStatefulEqn, testCreateFail)
{
    {
        std::string code = R"(
        function apply(input, output, t, data)
        end
        )";
        node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
        NLStatefulEqnBuilder builder{ log };
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math);
        auto funcs = builder.build_lua_functions(code, lua);
        EXPECT_FALSE(funcs);
    }
    {
        std::string code = R"(
            function update(input, t, data)
            end
        )";
        node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
        NLStatefulEqnBuilder builder{ log };
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math);
        auto funcs = builder.build_lua_functions(code, lua);
        EXPECT_FALSE(funcs);
    }
    {
        std::string code = R"()";
        node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
        NLStatefulEqnBuilder builder{ log };
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math);
        auto funcs = builder.build_lua_functions(code, lua);
        EXPECT_FALSE(funcs);
    }
}

TEST(testStatefulEqn, testCallsetup)
{
    std::string code = R"(
    function setup(data)
        data.ev.enabled = true
        data.ev.set = false
        data.ev.t = 1
        
        data.crossings:add(ZeroCrossDescriptor.new())
        descriptor = data.crossings[1]
        descriptor.value = 5
	    descriptor.in_port_id = 2
	    descriptor.type = CrossType.both
	    descriptor.last_value = CrossPosition.below
	    descriptor.current_value = CrossPosition.above
    end

    function apply(input, output, t, data)
    end

    function update(input, t, data)
    end
)";
    node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
    NLStatefulEqnBuilder builder{ log };
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    builder.AddUserTypes(lua);
    auto funcs = builder.build_lua_functions(code, lua);
    
    opt::NLStatefulEquationData data;
    ASSERT_TRUE(funcs);
    ASSERT_TRUE(funcs->setup);
    EXPECT_FALSE(data.ev.enabled);

    auto result = (*funcs->setup)(data);

    ASSERT_TRUE(result.valid());
    EXPECT_TRUE(data.ev.enabled);
    EXPECT_FALSE(data.ev.set);
    EXPECT_EQ(data.ev.t, 1);

    ASSERT_TRUE(data.crossings.size() == 1);
    EXPECT_EQ(data.crossings[0].value, 5);
    EXPECT_EQ(data.crossings[0].in_port_id, 2);
    EXPECT_EQ(data.crossings[0].type, opt::ZeroCrossDescriptor::CrossType::both);
    EXPECT_EQ(data.crossings[0].last_value, opt::ZeroCrossDescriptor::Position::below);
    EXPECT_EQ(data.crossings[0].current_value, opt::ZeroCrossDescriptor::Position::above);
}

TEST(testStatefulEqn, testCallapply)
{
    std::string code = R"(
    function apply(input, output, t, cdata)
        output[1] = t
        output[2] = cdata.ev.t
        output[3] = cdata.crossings[1].value
    end

    function update(input, t, data)
    end
)";
    node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
    NLStatefulEqnBuilder builder{ log };
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    builder.AddUserTypes(lua);
    auto funcs = builder.build_lua_functions(code, lua);

    ASSERT_TRUE(funcs);

    std::span<const double> inputs;
    std::vector<double> outputs{ 0, 0, 0};
    std::span<double> outputs_ref{ outputs };
    opt::NLStatefulEquationData data;
    data.ev.enabled = true;
    data.ev.t = 3;
    data.crossings.push_back(opt::ZeroCrossDescriptor{ 10 });
    opt::NLStatefulEquationDataCRef ref{ data.crossings, data.ev };
    LuaNLStatefulEquationDataCRef ref2{ &ref.crossings, &ref.ev };

    auto result = funcs->apply(inputs, outputs_ref, 5, ref2);

    ASSERT_TRUE(result.valid());
    EXPECT_EQ(outputs[0], 5);
    EXPECT_EQ(outputs[1], 3);
    EXPECT_EQ(outputs[2], 10);
}

TEST(testStatefulEqn, testCallUpdate)
{
    std::string code = R"(
    function apply(input, output, t, cdata)
    end

    function update(input, t, data)
        data.ev.t = input[1]
        data.crossings[1].value = input[2]
    end
)";
    node::logging::Logger log = node::logger(node::logging::LogCategory::Extension);
    NLStatefulEqnBuilder builder{ log };
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    builder.AddUserTypes(lua);
    auto funcs = builder.build_lua_functions(code, lua);

    ASSERT_TRUE(funcs);

    std::vector<double> inputs{ 5, 3 };
    std::span<const double> inputs_ref{ inputs };
    std::span<double> outputs{ };
    opt::NLStatefulEquationData data;
    data.ev.enabled = true;
    data.ev.t = 0;
    data.crossings.push_back({});
    opt::NLStatefulEquationDataRef ref{ data.crossings, data.ev };
    LuaNLStatefulEquationDataRef ref2{ &ref.crossings, &ref.ev };

    auto result = funcs->update(inputs_ref, 1, ref2);
    ASSERT_TRUE(result.valid());
    EXPECT_EQ(ref2.ev->t, 5);
    EXPECT_EQ(ref2.crossings->data()[0].value, 3);
}