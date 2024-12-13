#pragma once

#include <cstdint>
#include <vector>
#include "NodeModels/IdTypes.hpp"

namespace node
{

struct SceneManagerId
{
    SceneManagerId() = default;
    explicit SceneManagerId(int32_t value) : value{ value } {}
    bool operator==(const SceneManagerId&) const = default;
    int32_t value{};
};

struct SceneId
{
    SceneId() = default;
    SceneId(SceneManagerId manager, SubSceneId scene) : manager{ manager }, subscene{ scene } {}
    bool operator==(const SceneId&) const = default;
    SceneManagerId manager;
    SubSceneId subscene;
};
}

template <>
struct std::hash<node::SceneManagerId>
{
    std::size_t operator()(const node::SceneManagerId& k) const
    {
        return k.value;
    }
};

template <>
struct std::hash<node::SceneId>
{
    std::size_t operator()(const node::SceneId& k) const
    {
        size_t hash = 23;
        hash = hash * 31 + k.manager.value;
        hash = hash * 31 + k.subscene.value;
        return hash;
    }
};