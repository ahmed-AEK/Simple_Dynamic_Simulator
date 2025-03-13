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

typedef enum CBlockType
{
	CBlockTypeFunctional,
	CBlockTypeSubsystem,
	CBlockTypePort,
} CBlockType;

typedef struct CBlockTemplate
{
	CStr category;
	CStr template_name;
	CBlockType block_type;
	CFunctionalBlockData functional_data;
	CStr styler_name;
	CStylerPropertyVec style_properties;
} CBlockTemplate;

#ifdef __cplusplus
}
#endif