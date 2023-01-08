#ifndef vk_mem_alloc_h__
#define vk_mem_alloc_h__

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include "vk_mem_alloc_6_5_0.h"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
#include "vk_mem_alloc_6_4_0.h"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#include "vk_mem_alloc_6_3_0.h"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "vk_mem_alloc_6_0_0.h"
#endif

#endif // vk_mem_alloc_h__
