/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TIZEN_STICKER_PANEL_H__
#define __TIZEN_STICKER_PANEL_H__

#include <Elementary.h>
#include <tizen_error.h>
#include <bundle.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file sticker_panel.h
 * @brief Declares the API of the libsticker-panel library.
 */

/**
 * @addtogroup CAPI_PANEL_STICKER_MODULE
 * @{
 */

/**
 * @brief Definition for the sticker result type.
 * @since_tizen 2.4
 */
#define STICKER_PANEL_RESULT_TYPE "__STICKER_PANEL_RESULT_TYPE__"

/**
 * @brief Definition for the sticker result value.
 * @since_tizen 2.4
 */
#define STICKER_PANEL_RESULT_VALUE "__STICKER_PANEL_RESULT_VALUE__"

/**
 * @brief Definition for the sticker type : directory type for giving the directory path that includes sticker files.
 * @since_tizen 2.4
 */
#define STICKER_PANEL_RESULT_TYPE_DIRECTORY "__STICKER_PANEL_RESULT_TYPE_DIRECTORY__"

/**
 * @brief Definition for the sticker type : file type for giving the file path.
 * @since_tizen 2.4
 */
#define STICKER_PANEL_RESULT_TYPE_FILE "__STICKER_PANEL_RESULT_TYPE_FILE__"

/**
 * @brief Sticker panel handle.
 * @since_tizen 2.4
 */
typedef struct _sticker_panel *sticker_panel_h;

/**
 * @brief Called when an user selects and confirms sticker(s) to attach on the caller app.
 *
 * @since_tizen 2.4
 * @param[in] sticker_panel Sticker panel handler
 * @param[in] result bundle.\n
 *                   The sticker panel returns stickers loading a @a bundle\n
 *                   if directory, the sticker panel makes two bundles\n
 *                                 key : #STICKER_PANEL_RESULT_TYPE, value : #STICKER_PANEL_RESULT_TYPE_DIRECTORY\n
 *                                 key : #STICKER_PANEL_RESULT_VALUE, value : a directory path\n
 *                   if file, the sticker panel makes two bundle\n
 *                                 key : #STICKER_PANEL_RESULT_TYPE, value : #STICKER_PANEL_RESULT_TYPE_FILE\n
 *                                 key : #STICKER_PANEL_RESULT_VALUE, value : a file path\n
 * @param[in] user_data user data
 * @pre The callback must be registered using sticker_panel_set_result_cb()\n
 *      sticker_panel_show() must be called to invoke this callback.
 *
 * @see @ref CORE_LIB_BUNDLE_MODULE API bundle_get_str()
 * @see	sticker_panel_set_result_cb()
 * @see	sticker_panel_unset_result_cb()
 */
typedef void (*sticker_panel_result_cb)(sticker_panel_h sticker_panel, bundle *b, void *user_data);

/**
 * @brief Enumeration for values of the sticker panel response types.
 * @since_tizen 2.4
 */
typedef enum sticker_panel_error {
	STICKER_PANEL_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successfully handled */
	STICKER_PANEL_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Request is not valid, invalid parameter or invalid argument value */
	STICKER_PANEL_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Memory is not enough to handle a new request */
	STICKER_PANEL_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< Has no permission to attach contents */
	STICKER_PANEL_ERROR_ALREADY_EXISTS = TIZEN_ERROR_PANEL | 0x01, /**< There is already a panel in the conformant */
	STICKER_PANEL_ERROR_NOT_INITIALIZED = TIZEN_ERROR_PANEL | 0x02, /**< The panel is not initialized yet */
	STICKER_PANEL_ERROR_DB_FAILED = TIZEN_ERROR_PANEL | 0x04, /**< The panel cannot access DB */
	STICKER_PANEL_ERROR_ALREADY_REMOVED = TIZEN_ERROR_PANEL | 0x05, /**< The panel is already removed */
} sticker_panel_error_e;

/**
 * @brief Enumeration for values of the sticker panel view mode.
 * @since_tizen 2.4
 */
typedef enum sticker_panel_view_mode {
	STICKER_PANEL_VIEW_MODE_HALF = 1, /**< Half mode */
	STICKER_PANEL_VIEW_MODE_FULL, /**< Full mode */
} sticker_panel_view_mode_e;

/**
 * @brief Creates an sticker panel.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *          A conformant object can have only one @a sticker_panel_h.\n
 *          If a caller app try to add more than one sticker panel, it fails to add it.
 *
 * @param[in] conformant The caller's conformant
 * @param[out] sticker_panel Sticker panel handler
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #STICKER_PANEL_ERROR_PERMISSION_DENIED permission denied
 *
 * @see sticker_panel_destroy()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_show()
 * @see sticker_panel_hide()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static void _result_cb(sticker_panel_h sticker_panel, bundle *b, void *data)
 * {
 *   int i = 0;
 *   int length = 0;
 *   int ret = APP_CONTROL_ERROR_NONE;
 *   char type[1024];
 *   char value[1024];
 *
 *   if (!sticker_panel) {
 *     // Error handling
 *   }
 *
 *   if (!b) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_TYPE, &type);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!type) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_VALUE, &value);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!value) {
 *     // Error handling
 *   }
 *
 *   if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_DIRECTORY)) {
 *     // Routines for the directory
 *   } else if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_FILE)) {
 *     // Routines for the file
 *   } else {
 *     // Error handling
 *   }
 * }
 *
 * static int app_control(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = STICKER_PANEL_ERROR_NONE;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->conformant) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_create(ad->conformant, &ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_set_result_cb(ad->sticker_panel, _result_cb, NULL);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_show(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_create(Evas_Object *conformant, sticker_panel_h *sticker_panel);

/**
 * @brief Destroys the sticker panel.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *
 * @param[in] sticker_panel Sticker panel handler
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @see sticker_panel_create()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_show()
 * @see sticker_panel_hide()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static int app_terminate(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = 0;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->sticker_panel) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_hide(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_unset_result_cb(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_destroy(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *   ad->sticker_panel = NULL;
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_destroy(sticker_panel_h sticker_panel);

/**
 * @brief Sets the result callback that will be called when an user selects and confirms something to sticker in the sticker panel.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *          You can set only one callback function with this API.\n
 *          If you set multiple callbacks with this API,\n
 *          the last one is registered only.
 *
 * @param[in] sticker_panel Sticker panel handler
 * @param[in] result_cb Sticker panel result callback
 * @param[in] user_data User data
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @pre Calls sticker_panel_create() before calling this function.
 * @post The result_cb set with sticker_panel_set_result_cb() will be called after an user select something to attach.
 * @see sticker_panel_create()
 * @see sticker_panel_destroy()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_show()
 * @see sticker_panel_hide()
 * @see sticker_panel_result_cb
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static void _result_cb(sticker_panel_h sticker_panel, bundle *b, void *data)
 * {
 *   int i = 0;
 *   int length = 0;
 *   int ret = APP_CONTROL_ERROR_NONE;
 *   char type[1024];
 *   char value[1024];
 *
 *   if (!sticker_panel) {
 *     // Error handling
 *   }
 *
 *   if (!b) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_TYPE, &type);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!type) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_VALUE, &value);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!value) {
 *     // Error handling
 *   }
 *
 *   if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_DIRECTORY)) {
 *     // Routines for the directory
 *   } else if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_FILE)) {
 *     // Routines for the file
 *   } else {
 *     // Error handling
 *   }
 * }
 *
 * static int app_control(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = STICKER_PANEL_ERROR_NONE;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->conformant) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_create(ad->conformant, &ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_set_result_cb(ad->sticker_panel, _result_cb, NULL);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_show(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_set_result_cb(sticker_panel_h sticker_panel, sticker_panel_result_cb result_cb, void *user_data);

/**
 * @brief Unsets the result callback that will be called when an user selects and confirms something to sticker in the sticker panel.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *
 * @param[in] sticker_panel Sticker panel handler
 * @param[in] result_cb Sticker panel result callback
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @pre Calls sticker_panel_create() before calling this function.
 * @see sticker_panel_create()
 * @see sticker_panel_destroy()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_show()
 * @see sticker_panel_hide()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static int app_terminate(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = 0;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->sticker_panel) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_hide(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_unset_result_cb(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_destroy(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *   ad->sticker_panel = NULL;
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_unset_result_cb(sticker_panel_h sticker_panel);

/**
 * @brief Sets the view mode that has half and full modes.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *          You need to set a mode with this API before using sticker_panel_show().\n
 *          The default mode is the half mode.
 *
 * @param[in] sticker_panel Sticker panel handler
 * @param[in] view_mode Sticker panel view mode
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @pre Calls sticker_panel_create() before calling this function.
 * @see sticker_panel_create()
 * @see sticker_panel_destroy()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_show()
 * @see sticker_panel_hide()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static int app_control(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = STICKER_PANEL_ERROR_NONE;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->conformant) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_create(ad->conformant, &ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_set_view_mode(ad->sticker_panel, STICKER_PANEL_VIEW_MODE_HALF);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_show(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_set_view_mode(sticker_panel_h sticker_panel, sticker_panel_view_mode_e view_mode);

/**
 * @brief Shows the sticker panel, asynchronously.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *
 * @param[in] sticker_panel Sticker panel handler
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @pre Calls sticker_panel_create() before calling this function.
 * @see sticker_panel_create()
 * @see sticker_panel_destroy()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_hide()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static void _result_cb(sticker_panel_h sticker_panel, bundle *b, void *data)
 * {
 *   int i = 0;
 *   int length = 0;
 *   int ret = APP_CONTROL_ERROR_NONE;
 *   char type[1024];
 *   char value[1024];
 *
 *   if (!sticker_panel) {
 *     // Error handling
 *   }
 *
 *   if (!b) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_TYPE, &type);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!type) {
 *     // Error handling
 *   }
 *
 *   ret = bundle_get_str(b, STICKER_PANEL_RESULT_VALUE, &value);
 *   if (BUNDLE_ERROR_NONE != ret) {
 *     // Error handling
 *   } else if (!value) {
 *     // Error handling
 *   }
 *
 *   if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_DIRECTORY)) {
 *     // Routines for the directory
 *   } else if (!strcmp(type, STICKER_PANEL_RESULT_TYPE_FILE)) {
 *     // Routines for the file
 *   } else {
 *     // Error handling
 *   }
 * }
 *
 * static int app_control(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = STICKER_PANEL_ERROR_NONE;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->conformant) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_create(ad->conformant, &ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_set_result_cb(ad->sticker_panel, _result_cb, NULL);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_show(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_show(sticker_panel_h sticker_panel);

/**
 * @brief Hides the sticker panel, asynchronously.
 *
 * @since_tizen 2.4
 *
 * @privlevel public
 *
 * @remarks The caller app has to check the return value of this function.\n
 *
 * @param[in] sticker_panel Sticker panel handler
 * @return #STICKER_PANEL_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval #STICKER_PANEL_ERROR_NONE Successful
 * @retval #STICKER_PANEL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STICKER_PANEL_ERROR_ALREADY_REMOVED already removed
 *
 * @pre Calls sticker_panel_create() before calling this function.
 * @see sticker_panel_create()
 * @see sticker_panel_destroy()
 * @see sticker_panel_set_result_cb()
 * @see sticker_panel_unset_result_cb()
 * @see sticker_panel_set_view_mode()
 * @see sticker_panel_show()
 *
 * @par Example
 * @code
 * #include <sticker_panel.h>
 *
 * static struct appdata {
 *   Evas_Object *sticker_panel;
 *   Evas_Object *conformant;
 * };
 *
 * static int app_terminate(void *data)
 * {
 *   struct appdata *ad = data;
 *   int ret = 0;
 *
 *   if (!ad) {
 *     // Error handling
 *   }
 *
 *   if (!ad->sticker_panel) {
 *     // Error handling
 *   }
 *
 *   ret = sticker_panel_hide(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_unset_result_cb(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *
 *   ret = sticker_panel_destroy(ad->sticker_panel);
 *   if (STICKER_PANEL_ERROR_NONE != ret) {
 *      // Error handling
 *   }
 *   ad->sticker_panel = NULL;
 *
 * 	 return 0;
 * }
 *
 * @endcode
 */
int sticker_panel_hide(sticker_panel_h sticker_panel);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif // __TIZEN_STICKER_PANEL_H__

