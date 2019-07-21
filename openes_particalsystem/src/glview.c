/*
 * glview.c
 *
 *  Created on: Jul 21, 2019
 *      Author: dinglight
 */

#include "glview.h"

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


static GLuint LoadShader(GLenum type, const char *shaderSrc)
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
 * @brief create sharder program
 */
static GLuint CreateProgram(const char *vertexShaderSrc, const char *fragmentShaderSrc)
{
	/* Load the vertex/fragment shaders */
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertexShaderSrc);
	if (vertexShader == 0) {
		return 0;
	}
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
	if (fragmentShader == 0) {
		return 0;
	}
	/* Create the program object */
	GLuint program = glCreateProgram();
	if (program == 0) {
		return 0;
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
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * @brief Initializing function of GLView
 * @param[in] obj GLView object
 */
static void init_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	ad->initialized = false;

	if (!ad->initialized) {
		/* Vertex Shader Source */
		const char vShaderStr[] =
				"#version 300 es\n"
				"uniform float u_time;\n"
				"uniform vec3 u_centerPosition;\n"
				"layout(location = 0) in float a_lifetime;\n"
				"layout(location = 1) in vec3 a_startPosition;\n"
				"layout(location = 2) in vec3 a_endPosition;\n"
				"out float v_lifetime;\n"
				"void main()\n"
				"{\n"
				"  if (u_time <= a_lifetime) {\n"
				"    gl_Position.xyz = a_startPosition + (u_time * a_endPosition);\n"
				"    gl_Position.xyz += u_centerPosition;\n"
				"    gl_Position.w = 1.0;\n"
				"  } else {\n"
				"    gl_Position = vec4(0, 0, 0, 0);\n"
				"  }\n"
				"  v_lifetime = 1.0 - (u_time/a_lifetime);\n"
				"  v_lifetime = clamp(v_lifetime, 0.0, 1.0);\n"
				"  gl_PointSize = (v_lifetime * v_lifetime)*40.0;\n"
				"}";

		/* Fragment Shader Source */
		const char fShaderStr[] =
				"#version 300 es\n"
				"precision mediump float;\n"
				"uniform vec4 u_color;\n"
				"in float v_lifetime;\n"
				"out vec4 fragColor;"
				"\n"
				"void main()\n"
				"{\n"
				"  if(length(gl_PointCoord - vec2(0.5))>0.5)\n"
				"    discard;\n"
				"  fragColor = u_color;\n"
				"  fragColor.a *= v_lifetime;\n"
				"}";

		ad->program = CreateProgram(vShaderStr, fShaderStr);
		if (ad->program == 0) {
			return;
		}

		// get the uniform location
		ad->timeLoc = glGetUniformLocation(ad->program, "u_time");
		ad->centerPositionLoc = glGetUniformLocation(ad->program, "u_centerPosition");
		ad->colorLoc = glGetUniformLocation(ad->program, "u_color");

		// Fill in particle data array
		srand(0);

		for (int i = 0; i < NUM_PARTICLES; ++i) {
			float *particleData = &ad->particleData[i * PARTICLE_SIZE];
			// lifetime of particle
			(*particleData++) = ((float)(rand() % 10000)/10000.0f);
			// end position of particle
			(*particleData++) = ((float)(rand() % 10000)/5000.0f) - 1.0f;
			(*particleData++) = ((float)(rand() % 10000)/5000.0f) - 1.0f;
			(*particleData++) = ((float)(rand() % 10000)/5000.0f) - 1.0f;
			// start position of particle
			(*particleData++) = ((float)(rand() % 10000)/40000.0f) - 0.125f;
			(*particleData++) = ((float)(rand() % 10000)/40000.0f) - 0.125f;
			(*particleData++) = ((float)(rand() % 10000)/40000.0f) - 0.125f;
		}

		ad->time = 1.0f;

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

static void Update(appdata_s *ad, float deltaTime)
{
	ad->time += deltaTime;
	//glUseProgram(ad->program);
	if (ad->time >= 1.0f) {
		float centerPos[3];
		float color[4];
		ad->time = 0.0f;
		// Pick a new start location and color
		centerPos[0] = ((float)(rand() % 10000)/10000.0f) - 0.5f;
		centerPos[1] = ((float)(rand() % 10000)/10000.0f) - 0.5f;
		centerPos[2] = ((float)(rand() % 10000)/10000.0f) - 0.5f;
		glUniform3fv(ad->centerPositionLoc, 1, &centerPos[0]);

		// random color
		color[0] = ((float)(rand() % 10000)/20000.0f) + 0.5f;
		color[1] = ((float)(rand() % 10000)/20000.0f) + 0.5f;
		color[2] = ((float)(rand() % 10000)/20000.0f) + 0.5f;
		color[3] = 0.5f;
		glUniform4fv(ad->colorLoc, 1, &color[0]);
	}
	glUniform1f(ad->timeLoc, ad->time);
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

	Update(ad, 0.02f);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat), &ad->particleData[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat), &ad->particleData[1]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat), &ad->particleData[4]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Blend particales
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

	glFlush();
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

void create_glview(appdata_s *ad)
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
