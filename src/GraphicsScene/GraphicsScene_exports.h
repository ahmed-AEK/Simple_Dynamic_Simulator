#pragma once

#ifdef GRAPHICSSCENE_DYNAMIC
#ifdef GRAPHICSSCENE_EXPORTS
#define GRAPHICSSCENE_API __declspec(dllexport)
#else
#define GRAPHICSSCENE_API __declspec(dllimport)
#endif
#else
#define GRAPHICSSCENE_API 
#endif
