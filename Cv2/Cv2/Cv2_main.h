// PV227, Lesson 2: Shadow mapping
#pragma once

#include "../../Framework/PV227.h"

using namespace std;
using namespace PV227;

//---------------------
//----    SCENE    ----
//---------------------

// Shader programs
ShaderProgram notexture_program;
ShaderProgram texture_program;
ShaderProgram nolit_program;
ShaderProgram gen_shadow_program;
ShaderProgram display_shadow_texture_program;

// Geometries we use in this lecture
Geometry geom_cube;
Geometry geom_sphere;
Geometry geom_torus;
Geometry geom_teapot;
Geometry geom_fullscreen_quad;

// A camera that allows us to look at the object from different views
SimpleCamera the_camera;

// Textures
GLuint wood_tex;
GLuint lenna_tex;

// Data of our lights - positions, colors, etc.
PhongLightsData_UBO PhongLights_ubo;

// Data of our camera - view matrix, projection matrix, etc.
CameraData_UBO CameraData_ubo;

// Data of the camera that is used when rendering from the position of the light
glm::mat4 LightCameraProjection;			// Projection matrix of the camera
glm::mat4 LightCameraView;					// View matrix of the camera
CameraData_UBO LightCameraData_ubo;			// UBO with the data

// Shadow texture
GLuint ShadowFBO;			// Framebuffer object that is used when rendering into the shadow texture
GLuint ShadowTexture;		// Shadow texture
int ShadowTexSize = 1024;	// Width and height of the shadow texture
glm::mat4 ShadowMatrix;

// Data of our materials
MaterialData_UBO CubeMaterial_ubo;
MaterialData_UBO TorusMaterial_ubo;
MaterialData_UBO TeapotMaterial_ubo;
MaterialData_UBO FloorMaterial_ubo;
MaterialData_UBO LightMaterial_ubo;

// Data of the our objects (their position)
ModelData_UBO Cube1Model_ubo;
ModelData_UBO Cube2Model_ubo;
ModelData_UBO Cube3Model_ubo;
ModelData_UBO Torus1Model_ubo;
ModelData_UBO Torus2Model_ubo;
ModelData_UBO Torus3Model_ubo;
ModelData_UBO TeapotModel_ubo;
ModelData_UBO FloorModel_ubo;
ModelData_UBO LightModel_ubo;

// Functions that works with scene objects
void reload_shaders();
void init_scene();
void update_scene(int app_time_diff_ms);
void render_scene();

//-------------------
//----    GUI    ----
//-------------------

// Main AntTweakBar object
TwBar *the_gui;

const int DISPLAY_VIEW_FROM_MAIN_CAMERA = 0;
const int DISPLAY_VIEW_FROM_LIGHT = 1;
const int DISPLAY_SHADOW_TEXTURE = 2;

// Variables that are changed with GUI
float light_pos;
int current_display;

// Callbacks from the GUI
void TW_CALL reload(void *);

// Functions that works with GUI
void init_gui();

//---------------------------
//----    APPLICATION    ----

// Current window size
int win_width = 800;
int win_height = 600;

// Current time of the application in milliseconds, for animations
int app_time_ms = 0;
int last_glut_time = 0;
