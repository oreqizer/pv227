#include "Cv2_main.h"

//---------------------
//----    SCENE    ----
//---------------------

/// Reloads all shaders
void reload_shaders()
{
	// Program for objects without textures
	notexture_program.Init();
	notexture_program.AddVertexShader("Shaders/notexture_vertex.glsl");
	notexture_program.AddFragmentShader("Shaders/notexture_fragment.glsl");
	notexture_program.Link();

	// Program for objects with textures
	texture_program.Init();
	texture_program.AddVertexShader("Shaders/texture_vertex.glsl");
	texture_program.AddFragmentShader("Shaders/texture_fragment.glsl");
	texture_program.Link();

	// Program for objects without textures and without lighting
	nolit_program.Init();
	nolit_program.AddVertexShader("Shaders/nolit_vertex.glsl");
	nolit_program.AddFragmentShader("Shaders/nolit_fragment.glsl");
	nolit_program.Link();

	// Program for displaying shadow maps
	display_shadow_texture_program.Init();
	display_shadow_texture_program.AddVertexShader("Shaders/fullscreen_quad_vertex.glsl");
	display_shadow_texture_program.AddFragmentShader("Shaders/display_shadow_texture_fragment.glsl");
	display_shadow_texture_program.Link();

	// Task 3: Prepare special shaders for rendering into the shadow map
	//gen_shadow_program.Init();
	//gen_shadow_program.AddVertexShader(...);
	//gen_shadow_program.AddFragmentShader(...);
	//gen_shadow_program.Link();

	cout << "Shaders are reloaded" << endl;
}

/// Initializes all objects of our scene
void init_scene()
{
	//----------------------------------------------
	//--  Display some information about the OpenGL

	PrintOpenGLInfo();

	//----------------------------------------------
	//--  Load shaders

	reload_shaders();
	
	//----------------------------------------------
	//--  Prepare the lights

	PhongLights_ubo.Init();
	PhongLights_ubo.SetGlobalAmbient(glm::vec3(0.0f));

	//----------------------------------------------
	//--  Prepare materials

	CubeMaterial_ubo.Init();
	CubeMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f, 1.0f, 1.0f), true, 200.0f, 1.0f));
	CubeMaterial_ubo.UpdateOpenGLData();

	TorusMaterial_ubo.Init();
	TorusMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(0.0f, 0.0f, 1.0f), true, 200.0f, 1.0f));
	TorusMaterial_ubo.UpdateOpenGLData();

	TeapotMaterial_ubo.Init();
	TeapotMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f, 0.0f, 0.0f), true, 200.0f, 1.0f));
	TeapotMaterial_ubo.UpdateOpenGLData();

	FloorMaterial_ubo.Init();
	FloorMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(0.7f, 0.7f, 0.7f), false, 200.0f, 1.0f));
	FloorMaterial_ubo.UpdateOpenGLData();

	LightMaterial_ubo.Init();
	LightMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f), false, 0.0f, 1.0f));
	LightMaterial_ubo.UpdateOpenGLData();

	//----------------------------------------------
	//--  Prepare all cameras

	the_camera.SetCamera(-0.5f, -0.5f, 15.0f);
	CameraData_ubo.Init();

	// Task 1: Initialize LightCamera matrices
	//		Hint: Use glm::perspective, use 80 degrees as field of view, 2.0 as near plane, and 30.0 as far plane. Think about the value of aspect.
	LightCameraProjection = glm::perspective(glm::radians(80.0f), float(win_width) / float(win_height), 2.0f, 30.0f);
	LightCameraView = glm::mat4(1.0f);
	LightCameraData_ubo.Init();
	LightCameraData_ubo.SetProjection(LightCameraProjection);
	LightCameraData_ubo.SetCamera(LightCameraView);
	LightCameraData_ubo.UpdateOpenGLData();

	//----------------------------------------------
	//--  Positions of objects in the scene

	Cube1Model_ubo.Init();
	Cube1Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 1.0f, 0.0f)));
	Cube1Model_ubo.UpdateOpenGLData();

	Cube2Model_ubo.Init();
	Cube2Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 1.0f, -4.5f)));
	Cube2Model_ubo.UpdateOpenGLData();

	Cube3Model_ubo.Init();
	Cube3Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 1.0f, 4.5f)));
	Cube3Model_ubo.UpdateOpenGLData();

	Torus1Model_ubo.Init();
	Torus1Model_ubo.SetMatrix(glm::mat4(1.0f));			// The torus is animated
	Torus1Model_ubo.UpdateOpenGLData();

	Torus2Model_ubo.Init();
	Torus2Model_ubo.SetMatrix(glm::mat4(1.0f));			// The torus is animated
	Torus2Model_ubo.UpdateOpenGLData();

	Torus3Model_ubo.Init();
	Torus3Model_ubo.SetMatrix(glm::mat4(1.0f));			// The torus is animated
	Torus3Model_ubo.UpdateOpenGLData();

	TeapotModel_ubo.Init();
	TeapotModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	TeapotModel_ubo.UpdateOpenGLData();

	FloorModel_ubo.Init();
	FloorModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f)));
	FloorModel_ubo.UpdateOpenGLData();

	LightModel_ubo.Init();
	LightModel_ubo.SetMatrix(glm::mat4(1.0f));
	LightModel_ubo.UpdateOpenGLData();

	//----------------------------------------------
	//--  Prepare geometries

	geom_cube = CreateCube();
	geom_sphere = CreateSphere();
	geom_torus = CreateTorus();
	geom_teapot = CreateTeapot();
	geom_fullscreen_quad = CreateFullscreenQuad();

	//----------------------------------------------
	//--  Prepare textures

	wood_tex = CreateAndLoadTexture2D(L"../../textures/wood.jpg");
	glBindTexture(GL_TEXTURE_2D, wood_tex);
	SetTexture2DParameters(GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	lenna_tex = CreateAndLoadTexture2D(L"../../textures/Lenna.jpg");
	glBindTexture(GL_TEXTURE_2D, lenna_tex);
	SetTexture2DParameters(GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the shadow texture, allocate its memory, and set its basic parameters
	glGenTextures(1, &ShadowTexture);
	glBindTexture(GL_TEXTURE_2D, ShadowTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowTexSize, ShadowTexSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
	SetTexture2DParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//----------------------------------------------
	//--  Prepare framebuffers

	// Create the framebuffer for rendering into the shadow texture, and set the shadow texture to it
	glGenFramebuffers(1, &ShadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowTexture, 0);
	CheckFramebufferStatus("ShadowFBO");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// Updates the scene: performs animations, updates the data of the buffers, etc.
void update_scene(int app_time_diff_ms)
{
	glm::vec3 light_position = glm::vec3(
		15.0f * cosf(light_pos / 6.0f) * sinf(light_pos),
		15.0f * sinf(light_pos / 6.0f),
		15.0f * cosf(light_pos / 6.0f) * cosf(light_pos));

	// Data of the lights
	PhongLights_ubo.PhongLights.clear();
	PhongLights_ubo.PhongLights.push_back(PhongLight::CreateSpotLight(
		light_position,
		glm::vec3(0.04f, 0.04f, 0.04f),
		glm::vec3(0.7f, 0.7f, 0.7f),
		glm::vec3(0.7f, 0.7f, 0.7f),
		-glm::normalize(light_position), 20.0f, cosf(glm::radians(40.0f))));
	PhongLights_ubo.UpdateOpenGLData();

	LightModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), light_position) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	LightModel_ubo.UpdateOpenGLData();

	// Data of the main camera
	CameraData_ubo.SetProjection(
		glm::perspective(glm::radians(45.0f), float(win_width) / float(win_height), 0.1f, 1000.0f));
	CameraData_ubo.SetCamera(the_camera);
	CameraData_ubo.UpdateOpenGLData();

	// Data of the camera that is used when rendering from the light
	// Task 1: Update LightCamera matrices
	//		Hint: Use data of the first light from PhongLights_ubo.PhongLights
	auto lightFirst = PhongLights_ubo.PhongLights.front();
	auto lightFirstPosition = glm::vec3(lightFirst.position);

	LightCameraView = glm::lookAt(lightFirstPosition, lightFirstPosition + lightFirst.spot_direction, glm::vec3(0.0f, 1.0f, 0.0f));
	LightCameraData_ubo.SetCamera(LightCameraView);
	LightCameraData_ubo.UpdateOpenGLData();

	// Task 4: Compute and update shadow matrix
	//			Hint: glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z))
	//			Hint: glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z))
	// Task 7: Add additional translation matrix to the shadow matrix
	ShadowMatrix = glm::mat4(1.0f);

	// Animate the objects
	Torus1Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 1.5f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), float(app_time_ms) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	Torus2Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 1.5f, -4.5f)) *
		glm::rotate(glm::mat4(1.0f), float(app_time_ms) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	Torus3Model_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 1.5f, 4.5f)) *
		glm::rotate(glm::mat4(1.0f), float(app_time_ms) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	Torus1Model_ubo.UpdateOpenGLData();
	Torus2Model_ubo.UpdateOpenGLData();
	Torus3Model_ubo.UpdateOpenGLData();
}

void render_scene_once(bool genShadow = false)
{
	// Task 1: Do not render the light object when rendering from the light.
	//			Hint: add parameters to this function.
	// Task 3: Use special shaders for rendering into the shadow map
	//			Hint: add parameters to this function.
	// Task 4: Send shadow matrix into shaders
	//			Hint: To update uniform variable in shaders, use my_program.UniformMatrix4fv("shadow_matrix", 1, GL_FALSE, glm::value_ptr(ShadowMatrix));
	//				Call it when the program is active, don't forget to update uniform variables of all shaders.
	// Task 5: Send shadow texture ShadowTexture into shaders
	//			Hint: We use texture unit 1 for all shaders, so that the texture could be bound only once, at the beginning of this function
	// Task 7: Use glPolygonOffset
	//			Hint: Use glPolygonOffset(2.0f, 0.0f), and don't forget to glEnable/glDisable(GL_POLYGON_OFFSET_FILL)
	// Task 7: Cull front faces
	//			Hint: glEnable/glDisable(GL_CULL_FACE), glCullFace(GL_FRONT)
	// Task 8: Use GL_COMPARE_REF_TO_TEXTURE as GL_TEXTURE_COMPARE_MODE when using shadow texture
	//			Hint: It means call glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)
	//			Hint: Set also the compare function with glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL)

	// Render objects without any texture
	if (notexture_program.IsValid())
	{
		notexture_program.Use();
		notexture_program.UniformMatrix4fv("shadow_matrix", 1, GL_FALSE, glm::value_ptr(ShadowMatrix));

		// Floor
		FloorModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		FloorMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.BindVAO();
		geom_cube.Draw();

		// Teapot
		TeapotModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		TeapotMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_teapot.BindVAO();
		geom_teapot.Draw();

		// Tori
		TorusMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_torus.BindVAO();

		Torus1Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_torus.Draw();
		Torus2Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_torus.Draw();
		Torus3Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_torus.Draw();
	}

	// Render objects with textures
	if (texture_program.IsValid())
	{
		texture_program.Use();
		texture_program.UniformMatrix4fv("shadow_matrix", 1, GL_FALSE, glm::value_ptr(ShadowMatrix));

		// Cubes
		CubeMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.BindVAO();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_tex);

		Cube1Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cube.Draw();
		Cube2Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cube.Draw();
		Cube3Model_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cube.Draw();
	}

	// Render the light
	if (nolit_program.IsValid() && !genShadow)
	{
		nolit_program.Use();

		LightModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		LightMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_sphere.BindVAO();
		geom_sphere.Draw();
	}
}

/// Renders the whole frame
void render_scene()
{
	//--------------------------------------
	//--  Render into the shadow texture  --

	// Task 2: Render the scene into the shadow map
	//			Hint: Do the same as if you rendered into the main window, just change a FBO
	//				and associated parameters.
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
	glViewport(0, 0, ShadowTexSize, ShadowTexSize);

	// Clear the framebuffer, clear the color and depth
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	LightCameraData_ubo.BindBuffer(DEFAULT_CAMERA_BINDING);
	render_scene_once(true);

	//-----------------------------------
	//--  Render into the main window  --

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, win_width, win_height);

	// Clear the framebuffer, clear the color and depth
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	switch (current_display)
	{
	case DISPLAY_VIEW_FROM_MAIN_CAMERA:
		{
			// Set the data of the scene, like the camera and the lights. Use the main camera
			CameraData_ubo.BindBuffer(DEFAULT_CAMERA_BINDING);
			PhongLights_ubo.BindBuffer(DEFAULT_LIGHTS_BINDING);
			render_scene_once();
			break;
		}
	case DISPLAY_VIEW_FROM_LIGHT:
		{
			// Task 1: Render from the position of the light
			// Set the data of the scene, like the camera and the lights. Use the light camera
			LightCameraData_ubo.BindBuffer(DEFAULT_CAMERA_BINDING);
			PhongLights_ubo.BindBuffer(DEFAULT_LIGHTS_BINDING);
			render_scene_once(true);
			break;
		}
	case DISPLAY_SHADOW_TEXTURE:
		{
			// Task 2: Display the shadow map (see also the Task in the shader)
			//			Hint: Use glActiveTexture / glBindTexture as if it was any other texture.
			// Task 8: Use GL_NONE as GL_TEXTURE_COMPARE_MODE when using shadow texture
			//			Hint: It means call glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE)
			// Use a special shader and render the shadow texture in grayscale
			display_shadow_texture_program.Use();
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ShadowTexture);

			geom_fullscreen_quad.BindVAO();
			geom_fullscreen_quad.Draw();
			break;
		}
	}

	// Reset the VAO and the program
	glBindVertexArray(0);
	glUseProgram(0);
}

//-------------------
//----    GUI    ----
//-------------------

/// Callback when user clicks on Reload button
void TW_CALL reload(void *)
{
	reload_shaders();
}

void init_gui()
{
	light_pos = 1.0f;
	current_display = DISPLAY_VIEW_FROM_MAIN_CAMERA;

	// Initialize AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(win_width, win_height);
	TwGLUTModifiersFunc(glutGetModifiers);

	// Initialize GUI
	the_gui = TwNewBar("Parameters");
	TwAddButton(the_gui, "Reload", reload, nullptr, nullptr);
	TwAddVarRW(the_gui, "Light direction", TW_TYPE_FLOAT, &light_pos, "min=1 max=8 step=0.03");

	TwEnumVal displays[3] = {
		{DISPLAY_VIEW_FROM_MAIN_CAMERA, "From main camera"},
		{DISPLAY_VIEW_FROM_LIGHT, "From light"},
		{DISPLAY_SHADOW_TEXTURE, "Shadow texture"}
	};
	TwType displayType = TwDefineEnum("DisplayType", displays, 3);
	TwAddVarRW(the_gui, "Display", displayType, &current_display, nullptr);
}

//---------------------------
//----    APPLICATION    ----
//---------------------------

/// GLUT callback - when the user presses a key. See glutKeyboardFunc for more info
void on_keyboard_func(unsigned char key, int x, int y)
{
	// Inform AntTweakBar
	if (TwEventKeyboardGLUT(key, x, y))
		return;		// Already handled by AntTweakBar

	switch (key)
	{
	case 27:		// Escape key
		exit(0);
		break;
	}
}

/// GLUT callback - when the user presses a special key (like F1 key). See glutSpecialFunc for more info
void on_special_func(int key, int x, int y)
{
	// Inform AntTweakBar
	if (TwEventSpecialGLUT(key, x, y))
		return;		// Already handled by AntTweakBar
}

/// GLUT callback - when the user presses or releases a mouse button. See glutMouseFunc for more info
void on_mouse_func(int button, int state, int x, int y)
{
	// Inform AntTweakBar
	if (TwEventMouseButtonGLUT(button, state, x, y))
		return;		// Already handled by AntTweakBar

	the_camera.OnMouseFunc(button, state, x, y);
}

/// GLUT callback - when the user moves the mouse when holding a botton. See glutMotionFunc for more info
void on_motion_func(int x, int y)
{
	// Inform AntTweakBar
	if (TwEventMouseMotionGLUT(x, y))
		return;		// Already handled by AntTweakBar

	the_camera.OnMotionFunc(x, y);
}

/// GLUT callback - when the user moves the mouse when not holding a botton. See glutPassiveMotionFunc for more info
void on_passive_motion_func(int x, int y)
{
	// Inform AntTweakBar
	if (TwEventMouseMotionGLUT(x, y))
		return;		// Already handled by AntTweakBar
}

/// GLUT callback - when the window is resized. See glutReshapeFunc for more info
void on_reshape(int width, int height)
{
	win_width = width;
	win_height = height;
	
	glViewport(0, 0, win_width, win_height);
	
	// Inform AntTweakBar
	TwWindowSize(win_width, win_height);
}

/// GLUT callback - when our animation timer requests a new frame. See glutTimerFunc for more info
void on_timer(int)
{
	// Register the timer to be called again in 20 ms, and rerender the scene
	glutTimerFunc(20, on_timer, 0);
	glutPostRedisplay();
}

/// GLUT callback - when the window needs to be redrawn. See glutDisplayFunc for more info
void on_display()
{
	//--  Update all the data

	// Update the application time
	int current_glut_time = glutGet(GLUT_ELAPSED_TIME);
	int app_time_diff_ms = current_glut_time - last_glut_time;
	app_time_ms += app_time_diff_ms;
	last_glut_time = current_glut_time;

	// Update the scene
	update_scene(app_time_diff_ms);

	//--  Render the scene

	// Render the scene
	render_scene();
	
	// Render the GUI
	TwDraw();

	// Swaps the front and back buffer (double-buffering)
	glutSwapBuffers();
}

/// Callback function to be called when we make an error in OpenGL
void GLAPIENTRY simple_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	switch (type)			// Report only some errors
	{
	case GL_DEBUG_TYPE_ERROR:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << message << endl;		// Put the breakpoint here
		return;
	default:
		return;
	}
}

/// C main function :-)
int main(int argc, char ** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);		// Return from the main loop

	// Set OpenGL Context parameters
	glutInitContextVersion(4, 3);				// Use OpenGL 4.3
	glutInitContextProfile(GLUT_CORE_PROFILE);	// Use OpenGL core profile
	glutInitContextFlags(GLUT_DEBUG);			// Use OpenGL debug context

	// Initialize window
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("PV227 - Cv2: Shadow mapping");

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// Initialize DevIL library
	InitDevIL();

	// Set the OpenGL debug callback
	SetDebugCallback(simple_debug_callback);

	// Register GLUT callbacks
	glutDisplayFunc(on_display);
	glutReshapeFunc(on_reshape);
	glutTimerFunc(20, on_timer, 0);

	glutKeyboardFunc(on_keyboard_func);
	glutSpecialFunc(on_special_func);
	glutMouseFunc(on_mouse_func);
	glutMotionFunc(on_motion_func);
	glutPassiveMotionFunc(on_passive_motion_func);

	// Initialize OpenGL stuff
	init_gui();
	init_scene();

	// Everything is loaded and prepared, reset the application time
	last_glut_time = glutGet(GLUT_ELAPSED_TIME);
	app_time_ms = 0;

	// Run the main loop
	glutMainLoop();

	// Unload AntTweakBar
	TwTerminate();

	return 0;
}