#pragma once

#ifdef TOOLGUI_DYNAMIC
#ifdef TOOLGUI_EXPORTS
#define TOOLGUI_API __declspec(dllexport)
#else
#define TOOLGUI_API __declspec(dllimport)
#endif
#else
#define TOOLGUI_API 
#endif
