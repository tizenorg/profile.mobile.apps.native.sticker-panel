/*
 * Samsung API
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __ATTACK_PANEL_SAMPLE_H__
#define __ATTACK_PANEL_SAMPLE_H__

#include <dlog.h>

#define PATH_LEN 1024

#define BASE_WIDTH 1280
#define BASE_HEIGHT 720

#define RESULT_TYPE_DIRECTORY "__STICKER_PANEL_RESULT_TYPE_DIRECTORY__"
#define RESULT_TYPE_FILE "__STICKER_PANEL_RESULT_TYPE_FILE__"

typedef enum {
	ICON_INFO_TYPE_NONE = 0,
	ICON_INFO_TYPE_DIRECTORY = 1,
	ICON_INFO_TYPE_FILE = 2,
	ICON_INFO_TYPE_MAX,
} icon_info_type_e;


//log
#if !defined(_D)
#define _D(fmt, arg...) LOGD(fmt"\n", ##arg)
#endif

#if !defined(_W)
#define _W(fmt, arg...) LOGW(fmt"\n", ##arg)
#endif

#if !defined(_E)
#define _E(fmt, arg...) LOGE(fmt"\n", ##arg)
#endif

#define retv_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define ret_if(expr) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define goto_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> goto", #expr); \
		goto val; \
	} \
} while (0)


#endif
