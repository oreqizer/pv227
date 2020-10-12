#include "Cv1_main.h"

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

	// Task 5b: Create a new shader program with a vertex shader that enlarges the object
	//			along its normal. Use nolit_fragment.glsl as the fragment shader, it already
	//			render the object with a single color.
	expand_program.Init();
	expand_program.AddVertexShader("Shaders/expand_vertex.glsl");
	expand_program.AddFragmentShader("Shaders/nolit_fragment.glsl");
	expand_program.Link();

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

	SphereMaterial_ubo.Init();
	SphereMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f, 1.0f, 1.0f), true, 200.0f, 1.0f));
	SphereMaterial_ubo.UpdateOpenGLData();

	CylinderMaterial_ubo.Init();
	CylinderMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(0.0f, 1.0f, 0.0f), true, 200.0f, 1.0f));
	CylinderMaterial_ubo.UpdateOpenGLData();

	TeapotMaterial_ubo.Init();
	TeapotMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f, 0.0f, 0.0f), true, 200.0f, 1.0f));
	TeapotMaterial_ubo.UpdateOpenGLData();

	FloorMaterial_ubo.Init();
	FloorMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(0.7f, 0.7f, 0.7f), false, 200.0f, 1.0f));
	FloorMaterial_ubo.UpdateOpenGLData();

	LightMaterial_ubo.Init();
	LightMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(1.0f), false, 0.0f, 1.0f));
	LightMaterial_ubo.UpdateOpenGLData();

	BlackMaterial_ubo.Init();
	BlackMaterial_ubo.SetMaterial(PhongMaterial::CreateBasicMaterial(glm::vec3(0.0f), false, 0.0f, 1.0f));
	BlackMaterial_ubo.UpdateOpenGLData();

	//----------------------------------------------
	//--  Prepare all cameras

	the_camera.SetCamera(-0.5f, -0.5f, 15.0f);
	CameraData_ubo.Init();

	//----------------------------------------------
	//--  Positions of objects in the scene

	CubeModel_ubo.Init();
	CubeModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 1.0f, 0.0f)));
	CubeModel_ubo.UpdateOpenGLData();

	TorusModel_ubo.Init();
	TorusModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 4.0f)));
	TorusModel_ubo.UpdateOpenGLData();

	SphereModel_ubo.Init();
	SphereModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 0.0f)));
	SphereModel_ubo.UpdateOpenGLData();

	CylinderModel_ubo.Init();
	CylinderModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -4.0f)));
	CylinderModel_ubo.UpdateOpenGLData();

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
	geom_cylinder = CreateCylinder();
	geom_capsule = CreateCapsule();
	geom_teapot = CreateTeapot();

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
}

/// Updates the scene: performs animations, updates the data of the buffers, etc.
void update_scene(int app_time_diff_ms)
{
	glm::vec3 light_position = glm::vec3(
		cosf(light_pos / 6.0f) * sinf(light_pos),
		sinf(light_pos / 6.0f),
		cosf(light_pos / 6.0f) * cosf(light_pos));

	// Data of the lights
	PhongLights_ubo.PhongLights.clear();
	PhongLights_ubo.PhongLights.push_back(PhongLight::CreateDirectionalLight(
		light_position,
		glm::vec3(0.04f, 0.04f, 0.04f),
		glm::vec3(0.7f, 0.7f, 0.7f),
		glm::vec3(0.7f, 0.7f, 0.7f)));
	PhongLights_ubo.UpdateOpenGLData();

	LightModel_ubo.SetMatrix(
		glm::translate(glm::mat4(1.0f), light_position * 15.0f) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	LightModel_ubo.UpdateOpenGLData();

	// Data of the camera
	CameraData_ubo.SetProjection(
		glm::perspective(glm::radians(45.0f), float(win_width) / float(win_height), 0.1f, 1000.0f));
	CameraData_ubo.SetCamera(the_camera);
	CameraData_ubo.UpdateOpenGLData();
}

/// Renders the whole frame
void render_scene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set the clear values and clear the framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Set the data of the scene, common for all objects - the camera and the lights
	CameraData_ubo.BindBuffer(DEFAULT_CAMERA_BINDING);
	PhongLights_ubo.BindBuffer(DEFAULT_LIGHTS_BINDING);

	// Task 5b: Render each object twice, first only the back faces (i.e. cull away the front faces)
	//			with black material and enlarged, and then only the front faces (i.e. cull away the
	//			black faces) and in a standard way.
	//			Hint: Remember OpenGL functions glEnable(GL_CULL_FACE), glDisable(GL_CULL_FACE),
	//				glCullFace(GL_FRONT), and glCullFace(GL_BACK).
	glEnable(GL_CULL_FACE);

	// Render objects without any texture
	if (notexture_program.IsValid() && expand_program.IsValid())
	{
		// Floor
		FloorModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cube.BindVAO();

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.Draw();
		glCullFace(GL_BACK);
		notexture_program.Use();
		FloorMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.Draw();

		// Teapot
		TeapotModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_teapot.BindVAO();

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_teapot.Draw();
		glCullFace(GL_BACK);
		notexture_program.Use();
		TeapotMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_teapot.Draw();

		// Torus
		TorusModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_torus.BindVAO();

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_torus.Draw();
		glCullFace(GL_BACK);
		notexture_program.Use();
		TorusMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_torus.Draw();

		// Cylinder
		CylinderModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cylinder.BindVAO();

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cylinder.Draw();
		glCullFace(GL_BACK);
		notexture_program.Use();
		CylinderMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cylinder.Draw();
	}

	// Render objects with textures
	if (texture_program.IsValid())
	{
		// Cube
		CubeModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_cube.BindVAO();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, lenna_tex);

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.Draw();
		glCullFace(GL_BACK);
		texture_program.Use();
		CubeMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_cube.Draw();

		// Sphere
		SphereModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		geom_sphere.BindVAO();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_tex);

		glCullFace(GL_FRONT);
		expand_program.Use();
		BlackMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_sphere.Draw();
		glCullFace(GL_BACK);
		texture_program.Use();
		SphereMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_sphere.Draw();
	}

	glDisable(GL_CULL_FACE);

	// Render the light
	if (nolit_program.IsValid())
	{
		nolit_program.Use();

		LightModel_ubo.BindBuffer(DEFAULT_OBJECT_BINDING);
		LightMaterial_ubo.BindBuffer(DEFAULT_MATERIAL_BINDING);
		geom_sphere.BindVAO();
		geom_sphere.Draw();
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
	light_pos = 0.0f;

	// Initialize AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(win_width, win_height);
	TwGLUTModifiersFunc(glutGetModifiers);

	// Initialize GUI
	the_gui = TwNewBar("Parameters");
	TwAddButton(the_gui, "Reload", reload, nullptr, nullptr);
	TwAddVarRW(the_gui, "Light direction", TW_TYPE_FLOAT, &light_pos, "min=0 max=9.42 step=0.03");
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
	glutCreateWindow("PV227 - Cv1: Toon shading, cel shading");

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