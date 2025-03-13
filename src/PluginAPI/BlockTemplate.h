#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct CStr
{
	const char* data;
	size_t size;
} CStr;

typedef struct CStylerProperty
{
	CStr name;
	CStr value;
} CStylerProperty;

typedef struct CStylerPropertyVec
{
	const CStylerProperty* data;
	size_t size;
} CStylerPropertyVec;

typedef enum CBlockPropertyType
{
	CBlockPropertyTypeStr = 0,
	CBlockPropertyTypeFloat = 1,
	CBlockPropertyTypeInt = 2,
	CBlockPropertyTypeUInt = 3,
	CBlockPropertyTypeBool = 4
} CBlockPropertyType;

typedef struct CBlockProperty
{
	CStr name;
	CBlockPropertyType prop_type;
	union
	{
		int64_t int_val;
		uint64_t uint_val;
		unsigned char bool_val;
		double float_val;
		CStr str_val;
	};
} CBlockProperty;

typedef struct CBlockPropertyVec
{
	const CBlockProperty* data;
	size_t size;
} CBlockPropertyVec;

typedef struct CFunctionalBlockData
{
	CStr block_class;
	CBlockPropertyVec properties;
} CFunctionalBlockData;


typedef struct CSubsystemBlockData
{
	CStr URL;
	int32_t scene_id;
} CSubsystemBlockData;

typedef enum CSocketType
{
	CSocketTypeInput = 0,
	CSocketTypeOutput = 1
} CSocketType;

typedef struct CPortBlockData
{
	int32_t socket_id;
	CSocketType port_type;
} CPortBlockData;

typedef enum CBlockType
{
	None = 0,
	CBlockTypeFunctional = 1,
	CBlockTypeSubsystem = 2,
	CBlockTypePort = 3,
} CBlockType;

typedef struct CBlockTemplate
{
	CStr category;
	CStr template_name;
	CBlockType block_type;
	union
	{
		CFunctionalBlockData functional_data;
		CSubsystemBlockData subsystem_data;
		CPortBlockData port_data;
	};
	CStr styler_name;
	CStylerPropertyVec style_properties;
} CBlockTemplate;

#ifdef __cplusplus
}
#endif