// PV227, Lesson 1: Toon shading, Cel shading
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
ShaderProgram expand_program;

// Geometries we use in this lecture
Geometry geom_cube;
Geometry geom_sphere;
Geometry geom_torus;
Geometry geom_cylinder;
Geometry geom_capsule;
Geometry geom_teapot;

// A camera that allows us to look at the object from different views
SimpleCamera the_camera;

// Textures
GLuint wood_tex;
GLuint lenna_tex;

// Data of our lights - positions, colors, etc.
PhongLightsData_UBO PhongLights_ubo;

// Data of our camera - view matrix, projection matrix, etc.
CameraData_UBO CameraData_ubo;

// Data of our materials
MaterialData_UBO CubeMaterial_ubo;
MaterialData_UBO TorusMaterial_ubo;
MaterialData_UBO SphereMaterial_ubo;
MaterialData_UBO CylinderMaterial_ubo;
MaterialData_UBO TeapotMaterial_ubo;
MaterialData_UBO FloorMaterial_ubo;
MaterialData_UBO LightMaterial_ubo;
MaterialData_UBO BlackMaterial_ubo;

// Data of the our objects (their position)
ModelData_UBO CubeModel_ubo;
ModelData_UBO TorusModel_ubo;
ModelData_UBO SphereModel_ubo;
ModelData_UBO CylinderModel_ubo;
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

// Variables that are changed with GUI
float light_pos;

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
