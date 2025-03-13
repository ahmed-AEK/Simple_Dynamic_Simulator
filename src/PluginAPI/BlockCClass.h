#pragma once

#include <stddef.h>
#include <stdint.h>
#include "PluginAPI/BlockTemplate.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CSocketTypeVec
{
	const CSocketType* data;
	size_t size;
} CSocketTypeVec;

typedef struct CBlockClass CBlockClass;
typedef int(*sockets_callback)(void*, CSocketTypeVec);
typedef int(*default_propertise_cb)(void*, CBlockPropertyVec);
typedef int(*get_name_cb)(void*, CStr);

typedef struct CBlockClassVTable
{
	int (*destruct)(CBlockClass*);
	int (*get_name)(const CBlockClass*, get_name_cb, void*);
	int (*get_default_properies)(const CBlockClass*, default_propertise_cb, void*);
	int (*valiate_block_properties)(const CBlockClass*, const CBlockPropertyVec*);
	int (*calculate_sockets)(const CBlockClass*, const CBlockPropertyVec*, sockets_callback, void*);
	int (*get_block_type)(const CBlockClass*, const CBlockPropertyVec*);
} CBlockClassVTable;

typedef struct CBlockClass
{
	const CBlockClassVTable* vtbl;
	void* data;
} CBlockClass;

#ifdef __cplusplus
}
#endif