/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "glviewexample.h"
/*
 * The file Elementary_GL_Helpers.h provies some convenience functions
 * that ease the use of OpenGL within Elementary application.
 */
#include <Elementary_GL_Helpers.h>

/*
 * ELEMENTARY_GLVIEW_GLOBAL_DEFINE() is
 * #define ELEMENTARY_GLVIEW_GLOBAL_DEFINE() \
 *  Evas_GL_API *__evas_gl_glapi = NULL;
 */
ELEMENTARY_GLVIEW_GLOBAL_DEFINE();

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *glview;
	Ecore_Animator *ani;
	int glview_h, glview_w;

	/* GL related data here... */
	unsigned int program;

	Eina_Bool initialized;
} appdata_s;

/* Vertex Shader Source */
static const char vShaderStrshaderSrc[] =
		"#version 300 es\n"
		"layout(location = 0) in vec4 aPos;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = aPos;\n"
		"}";

/* Fragment Shader Source */
static const char fShaderStr[] =
		"#version 300 es\n"
		"precision mediump float;\n"
		"out vec4 fragColor;\n"
		"\n"
		"void main (void)\n"
		"{\n"
		"    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}";

static const float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

/*
 * @brief Hide the window when back button is pressed
 * @param[in] data App data
 * @param[in] obj Elm Window object
 * @param[in] event_info Not use
 */
static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hidden state. */
	elm_win_lower(ad->win);
}

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;
	// Create the shader object
	shader = glCreateShader(type);
	if (shader == 0) {
		return 0;
	}
	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);
	// Compile the shader
	glCompileShader(shader);
	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char *infoLog = malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			dlog_print(DLOG_ERROR, LOG_TAG, "Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

/*
 * @brief Initialize vertex & fragment shaders
 * @param[in] obj GLView object
 */
static void init_shaders(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Load the vertex/fragment shaders */
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStrshaderSrc);
	if (vertexShader == 0) {
		return;
	}
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);
	if (fragmentShader == 0) {
		return;
	}
	/* Create the program object */
	GLuint program = glCreateProgram();
	if (program == 0) {
		return;
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// Link the program
	glLinkProgram(program);
	// Check the link status
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char *infoLog = malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
			dlog_print(DLOG_ERROR, LOG_TAG, "Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(program);
		return;
	}
	/* Store the program object */
	ad->program = program;

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

/*
 * @brief Callback function to be invoked when size of glview is resized
 * @param[in] obj GLView object
 */
static void resize_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Get size of GLView object for setting Viewport*/
	elm_glview_size_get(obj, &ad->glview_w, &ad->glview_h);

	glViewport(0, 0, ad->glview_w, ad->glview_h);
}

/*
 * @brief Drawing function of GLView
 * @param[in] obj GLView object
 */
static void draw_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	// Clear the color buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the program object
	glUseProgram(ad->program);
	// Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();
}

/*
 * @brief Initializing function of GLView
 * @param[in] obj GLView object
 */
static void init_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	if (!ad->initialized) {
		init_shaders(obj);
		ad->initialized = EINA_TRUE;
	}
}

/*
 * @brief Callback function to be invoked when glview object is deleted
 * @param[in] obj GLView object
 */
static void del_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Release resources. */
	glDeleteProgram(ad->program);

	evas_object_data_del((Evas_Object*) obj, "ad");
}

/*
 * @brief Callback function to be invoked when glview object is deleted
 *        Delete a animator
 */
static void del_anim(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	ecore_animator_del(ad->ani);
}

/*
 * @brief Animator makes GLView to draw new frame
 * param[in] data GLView object
 */
static Eina_Bool anim(void *data)
{
	elm_glview_changed_set(data);
	return EINA_TRUE;
}

static void create_glview(appdata_s *ad)
{
	Evas_Object *glview = elm_glview_add(ad->conform);

	/*
	 * ELEMENTARY_GLVIEW_GLOBAL_USE() is
	 * #define ELEMENTARY_GLVIEW_USE(glview) \
	 *  Evas_GL_API *__evas_gl_glapi = elm_glview_gl_api_get(glview);
	 */
	ELEMENTARY_GLVIEW_GLOBAL_USE(glview);
	evas_object_size_hint_align_set(glview, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(glview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/*
	 * Request a surface with a depth buffer
	 *
	 * To use the Direct Rendering mode, set the same option values (depth, stencil, and MSAA)
	 * to a rendering engine and a GLView object.
	 * You can set the option values to a rendering engine
	 * using the elm_config_accel_preference_set() function and
	 * to a GLView object using the elm_glview_mode_set() function.
	 * If the GLView object option values are bigger or higher than the rendering engine's,
	 * the Direct Rendering mode is disabled.
	 */
	elm_glview_mode_set(glview, ELM_GLVIEW_DEPTH | ELM_GLVIEW_DIRECT | ELM_GLVIEW_CLIENT_SIDE_ROTATION);

	/*
	 * The resize policy tells GLView what to do with the surface when it
	 * resizes. ELM_GLVIEW_RESIZE_POLICY_RECREATE will tell it to
	 * destroy the current surface and recreate it to the new size.
	 */
	elm_glview_resize_policy_set(glview, ELM_GLVIEW_RESIZE_POLICY_RECREATE);

	/*
	 * The render policy sets how GLView should render GL code.
	 * ELM_GLVIEW_RENDER_POLICY_ON_DEMAND will have the GL callback
	 * called only when the object is visible.
	 * ELM_GLVIEW_RENDER_POLICY_ALWAYS would cause the callback to be
	 * called even if the object were hidden.
	 */
	elm_glview_render_policy_set(glview, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);

	/* The initialize callback function gets registered here */
	elm_glview_init_func_set(glview, init_glview);

	/* The delete callback function gets registered here */
	elm_glview_del_func_set(glview, del_glview);

	/* The resize callback function gets registered here */
	elm_glview_resize_func_set(glview, resize_glview);

	/* The render callback function gets registered here */
	elm_glview_render_func_set(glview, draw_glview);

	/* Add the GLView to the conformant and show it */
	elm_object_content_set(ad->conform, glview);
	evas_object_show(glview);

	elm_object_focus_set(glview, EINA_TRUE);
	evas_object_data_set(glview, "ad", ad);

	ad->glview = glview;

	// crate ani
	/* This adds an animator so that the app will regularly
	 * trigger updates of the GLView using elm_glview_changed_set().
	 *
	 * NOTE: If you delete GL, this animator will keep running trying to access
	 * GL so this animator needs to be deleted with ecore_animator_del().
	 */
	ad->ani = ecore_animator_add(anim, ad->glview);
	evas_object_event_callback_add(ad->glview, EVAS_CALLBACK_DEL, del_anim, ad);
}

static void
win_delete_request_cb(void *data , Evas_Object *obj , void *event_info)
{
	ui_app_exit();
}

/*
 * @brief Create a conform
 * param[in] ad app data
 */
static void create_conform(appdata_s *ad)
{
	Evas_Object *conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	evas_object_show(conform);
	ad->conform = conform;
}

/*
 * @brief Create a window
 * param[in] name Window name
 */
static void create_win(appdata_s *ad)
{
	Evas_Object *win;

	/*
	 * To use the Direct Rendering mode of GLView,
	 * set the same option values (depth, stencil, and MSAA)
     * to a rendering engine and a GLView object.
	 */
	elm_config_accel_preference_set("opengl:depth");
	win = elm_win_util_standard_add("mysss", "OpenGL example: Tea pot");

	if (!win)
		return;

	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(win, rots, 4);
	}
	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	elm_win_conformant_set(win, EINA_TRUE);

	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);

	ad->win = win;
}

static bool app_create(void *data)
{
	appdata_s *ad = data;

	if (!data)
		return false;

	/* Create the window */
	create_win(ad);
	// Create conformant
	create_conform(ad);

	/* Create GLView and animation*/
	create_glview(ad);

	evas_object_show(ad->win);

	/* Return true: the main loop will now start running */
	return true;
}

static void app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
	dlog_print(DLOG_DEBUG, LOG_TAG, "app_control");
}

static void app_pause(void *data)
{
	appdata_s *ad = data;
	dlog_print(DLOG_DEBUG, LOG_TAG, "app_pause");
	/*
	 * When app is paused,
	 * Freeze animator for power saving
	 */
	ecore_animator_freeze(ad->ani);
}

static void app_resume(void *data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "app_resume");

	appdata_s *ad = data;
	/* When app is resumed, thaw animator */
	ecore_animator_thaw(ad->ani);
}

static void app_terminate(void *data)
{
	/* Release all resources. */
	dlog_print(DLOG_DEBUG, LOG_TAG, "app_terminate");
}

int main(int argc, char *argv[])
{
	int ret = 0;
	appdata_s ad = { NULL, };
	ui_app_lifecycle_callback_s event_callback = {NULL,};

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "The application failed to start, and returned %d", ret);

	return ret;
}
