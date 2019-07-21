# Vertex Buffer Object

### init_glview
```
/*
 * @brief Initializing function of GLView
 * @param[in] obj GLView object
 */
static void init_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	if (!ad->initialized) {
        /* Vertex Shader Source */
        const char vShaderStr[] =
                "#version 300 es\n"
                "layout(location = 0) in vec4 aPos;\n"
                //"layout(location = 1) in float aSize;\n"
                "void main()\n"
                "{\n"
                "    gl_Position = aPos;\n"
                "    gl_PointSize = 50.0f;\n"
                "}";

        /* Fragment Shader Source */
        const char fShaderStr[] =
                "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 fragColor;\n"
                "\n"
                "void main (void)\n"
                "{\n"
                "    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                "}";

        ad->program = CreateProgram(vShaderStr, fShaderStr);
        if (ad->program == 0) {
            return;
        }
		const float vertices[] = {
			    -0.5f, -0.5f, 0.0f,
			     0.5f, -0.5f, 0.0f,
			     0.0f,  0.5f, 0.0f
		};
		// Generate VBO and load the VBO with data
		glGenBuffers(1, &ad->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, ad->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		ad->initialized = EINA_TRUE;
	}
}
```

### del_glview
```
/*
 * @brief Callback function to be invoked when glview object is deleted
 * @param[in] obj GLView object
 */
static void del_glview(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Release resources. */
	glDeleteBuffers(1, &ad->vbo);
	glDeleteProgram(ad->program);

	evas_object_data_del((Evas_Object*) obj, "ad");
}
```

### draw_glview
```
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

	// draw with vbo
	glBindBuffer(GL_ARRAY_BUFFER, ad->vbo);

	// Load the vertex data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

	glDrawArrays(GL_POINTS, 0, 3);

	glFlush();

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	//DrawWithoutVBOs();
	DrawWithVBOs(ad);
}
```
