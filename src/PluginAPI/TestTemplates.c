#include "PluginAPI/BlockTemplate.h"
#include "PluginAPI/BlockCClass.h"

#include "string.h"

static const CBlockProperty sum_block_properties[] =
{
    {.name = {.data = "Multiplier", .size = sizeof("Multiplier") - 1}, .prop_type = CBlockPropertyTypeFloat, .float_val = 1.0},
};

static const CFunctionalBlockData sum_block_functional_data =
{
    .block_class = {.data = "Gain", .size = sizeof("Gain") - 1},
    .properties = {
        .data = sum_block_properties,
        .size = sizeof(sum_block_properties) / sizeof(CBlockProperty)
    }
};

static const CBlockTemplate sum_block_template =
{
    .category = {.data = "Math", .size = sizeof("Math") - 1},
    .template_name = {.data = "Sum", .size = sizeof("Sum") - 1},
    .block_type = CBlockTypeFunctional,
    .styler_name = {.data = "Sum", .size = sizeof("Sum") - 1},
    .functional_data = {
        .block_class = {.data = "Gain", .size = sizeof("Gain") - 1},
        .properties = {
            .data = sum_block_properties,
            .size = sizeof(sum_block_properties) / sizeof(CBlockProperty)
        }
    }
};

const CBlockTemplate* make_sum_template()
{
	return &sum_block_template;
}

static int sum_desetroy(CBlockClass* cls)
{
    if (!cls)
    {
        return 1;
    }
    cls->data = NULL;
    return 0;
}

static int sum_get_name(const CBlockClass* cls, get_name_cb cb, void* context)
{
    static const CStr s = { .data = "Sum", .size = sizeof("Sum") - 1 };
    cb(context, s);
    return 0;
}

static int sum_get_default_properties(const CBlockClass* cls, default_propertise_cb cb, void* context)
{
    cb(context, sum_block_functional_data.properties);
    return 0;
}

static int sum_valiate_block_properties(const CBlockClass* cls, const CBlockPropertyVec* vec)
{
    return 0;
}

static int sum_calculate_sockets(const CBlockClass* cls, const CBlockPropertyVec* vec, sockets_callback cb, void* context)
{
    static const CSocketType sockets[] = { CSocketTypeInput, CSocketTypeOutput };
    cb(context, (CSocketTypeVec){ sockets, sizeof(sockets) / sizeof(CSocketType) });
    return 0;
}

static int sum_get_block_type(const CBlockClass* cls, const CBlockPropertyVec* vec)
{
    return 0;
}

CBlockClass make_sum_class()
{
    static CBlockClassVTable vtbl =
    {
        .destruct = sum_desetroy,
        .get_name = sum_get_name,
        .get_default_properies = sum_get_default_properties,
        .valiate_block_properties = sum_valiate_block_properties,
        .calculate_sockets = sum_calculate_sockets,
        .get_block_type = sum_get_block_type,
    };

    CBlockClass block_cls = { .vtbl = &vtbl, .data = (void*)1 };
    return block_cls;
}
