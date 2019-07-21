#ifndef __openes_particalsystem_H__
#define __openes_particalsystem_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "openes_particalsystem"

#if !defined(PACKAGE)
#define PACKAGE "org.example.openes_particalsystem"
#endif

#define NUM_PARTICLES 1000
#define PARTICLE_SIZE 7

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *glview;
	Ecore_Animator *ani;
	int glview_h, glview_w;

	/* GL related data here... */
	GLuint program;
	GLuint vbo;    // vertex buffer object

	GLint centerPositionLoc;
	GLint colorLoc;
	GLint timeLoc;

	// particel vertex data
	float particleData[NUM_PARTICLES * PARTICLE_SIZE];
	float time;

	Eina_Bool initialized;
} appdata_s;

#endif /* __openes_particalsystem_H__ */
