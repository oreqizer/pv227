#pragma once
#ifndef INCLUDED_PV227_UBOS_H
#define INCLUDED_PV227_UBOS_H

#include "PV227_Basics.h"

// This file contains classes which store data into OpenGL uniform buffer objects
// which we use in our lessons.
//
// These classes have several common methods:
//	- The classes have a default constructor which does not create any OpenGL object, because OpenGL context may not be initialized yet.
//	- The classes do not have any destructor that would automatically destroy OpenGL objects, since OpenGL may be already unloaded.
//	- The classes have Init method which initializes OpenGL objects. Call it after OpenGL context is created, i.e. after creating a window.
//	- The classes have Destroy method which destroys all OpenGL objects. Call it before OpenGL context is unloaded, i.e. before deleting a window.
//	- The classes have GetBuffer method which returns the ID of the UBO which is used.
//	- The classes have BindBuffer method which binds the UBO to given binding point.
//	- The classes have UpdateOpenGLData method which copies the data from CPU to OpenGL.
//
//	- The classes can be initiated to contain the data on a single object, or for a set multiple objects (e.g. for instancing). Also,
//		the data can be stored into a uniform buffer or shader storage buffer. This is all set up in Init method.
//
// Example of how to use these classes:
//		// 1) Define a global variable
//		CameraData_UBO CameraData_ubo;
//		// 2) In init function, initialize all OpenGL objects
//		CameraData_ubo.Init();
//		// 3) In update function, set all data that changed, and then copy them to OpenGL buffers
//		CameraData_ubo.SetData(...);
//		CameraData_ubo.UpdateOpenGLData();
//		// 4) In draw function, bind the buffers to the proper binding point
//		CameraData_ubo.BindBuffer(1);
//
// GLSL code of the layout of the data which is used by the classes is also provided, it is places
// after the C++ code of the class.

namespace PV227
{

	//---------------------------
	//----    CAMERA DATA    ----
	//---------------------------

	/// CameraData_UBO contains the data of a camera, i.e. a projection matrix, a view matrix, and an eye position.
	/// The camera may be a single, or a set of multiple cameras.
	class CameraData_UBO
	{
	public:
		struct SingleCameraData
		{
			glm::mat4 projection;
			glm::mat4 projection_inv;
			glm::mat4 view;
			glm::mat4 view_inv;
			glm::mat3x4 view_it;		// Don't forget the padding
			glm::vec4 eye_position;		// Don't forget the padding
		};

	private:
		std::vector<SingleCameraData> data;
		GLuint buffer;
		GLenum target;

	public:
		// Common methods, see the comment at the beginning of this file for more info.
		CameraData_UBO();
		void Destroy();
		GLuint GetBuffer() const;
		void BindBuffer(GLuint index) const;
		void UpdateOpenGLData() const;

		/// Creates all OpenGL objects, allocates the UBO to be able to contain given number of cameras.
		void Init(size_t count = 1, GLenum target = GL_UNIFORM_BUFFER);

		/// Sets the data of the projection (projection matrix and its inverse)
		void SetProjection(const glm::mat4 &projection_matrix);
		/// Sets the data of the camera (view matrix, its derivatices, and eye position)
		void SetCamera(const SimpleCamera &camera);
		void SetCamera(const glm::mat4 &view_matrix);

		/// Sets the data of the projection (projection matrix and its inverse) of a given camera
		void SetProjection(int idx, const glm::mat4 &projection_matrix);
		/// Sets the data of the camera (view matrix, its derivatices, and eye position) of a given camera
		void SetCamera(int idx, const SimpleCamera &camera);
		void SetCamera(int idx, const glm::mat4 &view_matrix);
	};

	/* Use this code in shaders

	- single camera

layout (std140) uniform CameraData
{
	mat4 projection;		// Projection matrix
	mat4 projection_inv;	// Inverse of the projection matrix
	mat4 view;				// View matrix
	mat4 view_inv;			// Inverse of the view matrix
	mat3 view_it;			// Inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// Position of the eye in world space
};

	- or a set of multiple cameras

struct SingleCameraData
{
	mat4 projection;		// Projection matrix
	mat4 projection_inv;	// Inverse of the projection matrix
	mat4 view;				// View matrix
	mat4 view_inv;			// Inverse of the view matrix
	mat3 view_it;			// Inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// Position of the eye in world space
};
layout (std140) uniform CameraData
{
	SingleCameraData cameras[#count];
};

	*/

	//--------------------------
	//----    MODEL DATA    ----
	//--------------------------

	/// Contains the model matrix, which defines the position of the object in the world, and derivations of this matrix.
	class ModelData_UBO
	{
	public:
		struct SingleModelData
		{
			glm::mat4 model;
			glm::mat4 model_inv;
			glm::mat3x4 model_it;
		};

	private:
		std::vector<SingleModelData> data;
		GLuint buffer;
		GLenum target;

	public:
		// Common methods, see the comment at the beginning of this file for more info.
		ModelData_UBO();
		void Destroy();
		GLuint GetBuffer();
		void BindBuffer(GLuint index);
		void UpdateOpenGLData();

		/// Creates all OpenGL objects, allocates the UBO to be able to contain given number of objects.
		void Init(size_t count = 1, GLenum target = GL_UNIFORM_BUFFER);

		/// Sets the model matrix of the object and its derivations
		void SetMatrix(const glm::mat4 &model);
		/// Sets the model matrix of a given object and its derivations
		void SetMatrix(int idx, const glm::mat4 &model);
	};

	/* Use this code in shaders

	- single object

layout (std140) uniform ModelData
{
	mat4 model;			// Model matrix
	mat4 model_inv;		// Inverse of the model matrix
	mat3 model_it;		// Inverse of the transpose of the top-left part 3x3 of the model matrix
};

	- or a set of multiple objects

struct SingleModelData
{
	mat4 model;			// Model matrix
	mat4 model_inv;		// Inverse of the model matrix
	mat3 model_it;		// Inverse of the transpose of the top-left part 3x3 of the model matrix
};
layout (std140) uniform ModelData
{
	SingleModelData models[#count];
};

	*/

	//---------------------------------
	//----    PHONG LIGHTS DATA    ----
	//---------------------------------

	/// Basic structure which contains all data of a Phong light. It uses the same layout as
	/// its GLSL counterpart.
	struct PhongLight
	{
		/// Position of the light. position.w is one for point lights and spot lights, and zero for directional lights.
		glm::vec4 position;
		/// Ambient part of the color of the light
		glm::vec3 ambient;				float padding1;
		/// Diffuse part of the color of the light
		glm::vec3 diffuse;				float padding2;
		/// Specular part of the color of the light
		glm::vec3 specular;				float padding3;
		/// Direction of the spot light, irrelevant for point lights and directional lights
		glm::vec3 spot_direction;
		/// Spot exponent of the spot light, irrelevant for point lights and directional lights
		float spot_exponent;
		/// Cosine of the spot light's cutoff angle, -1 point lights, irrelevant for directional lights
		float spot_cos_cutoff;
		/// Constant, linear and quadratic attenutation of spot lights and point lights, irrelevant for directional lights
		float atten_constant;		// For no attenuation, set this to 1
		float atten_linear;			// For no attenuation, set this to 0
		float atten_quadratic;		// For no attenuation, set this to 0

		/// Constructor for all types of the light
		static PhongLight CreateLight(
			const glm::vec4 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular,
			const glm::vec3 &spot_direction,
			float spot_exponent,
			float spot_cos_cutoff,
			float atten_constant,
			float atten_linear,
			float atten_quadratic);

		/// Constructor for point lights, sets all attributes that are not specified
		static PhongLight CreatePointLight(
			const glm::vec3 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular);
		static PhongLight CreatePointLight(
			const glm::vec3 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular,
			float atten_constant,
			float atten_linear,
			float atten_quadratic);

		/// Constructor for a directional light, sets all attributes that are not specified
		static PhongLight CreateDirectionalLight(
			const glm::vec3 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular);

		/// Constructor for spot lights, sets all attributes that are not specified
		static PhongLight CreateSpotLight(
			const glm::vec3 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular,
			const glm::vec3 &spot_direction,
			float spot_exponent,
			float spot_cos_cutoff);
		static PhongLight CreateSpotLight(
			const glm::vec3 &position,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular,
			const glm::vec3 &spot_direction,
			float spot_exponent,
			float spot_cos_cutoff,
			float atten_constant,
			float atten_linear,
			float atten_quadratic);

		/// Constructor for a head light. Head light is a point light whose position is at the position of the camera
		static PhongLight CreateHeadLight(
			const glm::mat4 &view_matrix,
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular);
	};

	/// This class contains the data of all Phong lights in the scene. It also contains the data
	/// of the global scene ambient light, as do the old OpenGL.
	///
	/// This class is prepared for eight lights by default, but this number may be increased.
	/// To change the number of the lights, change the number of lights in GLSL, and call Init
	/// with this number of the lights.
	///
	/// The data of individual lights are public (to ease prototyping), in PhongLights array.
	/// The size of this array needs not to be the same as the maximum number of supported lights
	/// for which the UBO is prepared. Only the data of the lights that are used are copied into
	/// OpenGL, up to the maximum number of supported lights.
	class PhongLightsData_UBO
	{
	public:
		struct PhongLightsDataHeader
		{
			glm::vec3 global_ambient_color;
			int lights_count;
		};

	private:
		PhongLightsDataHeader header;
		GLuint buffer;
		size_t max_lights;
		GLenum target;

	public:
		// Common methods, see the comment at the beginning of this file for more info.
		PhongLightsData_UBO();
		void Destroy();
		GLuint GetBuffer();
		void BindBuffer(GLuint index);
		void UpdateOpenGLData();

		/// Data of all lights
		std::vector<PhongLight> PhongLights;

		/// Creates all OpenGL objects, allocates the UBO to be able to contain given number of lights.
		void Init(size_t max_lights = 8, GLenum target = GL_UNIFORM_BUFFER);

		/// Set the intensity of the global ambient light
		void SetGlobalAmbient(const glm::vec3 &global_ambient_color);
	};

	/* Use this code in shaders

struct PhongLight
{
	// See the C++ code for the documentation to individual attributes
	vec4 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 spot_direction;
	float spot_exponent;
	float spot_cos_cutoff;
	float atten_constant;
	float atten_linear;
	float atten_quadratic;
};
layout (std140) uniform PhongLightsData
{
	vec3 global_ambient_color;
	int lights_count;
	PhongLight lights[#count];
};

// Evaluates the lighting of one Phong light
// light	.. [in] parameters of the light that is evaluated
// amb		.. [out] result, ambient part
// dif		.. [out] result, diffuse part
// spe		.. [out] result, specular part
// norm		.. [in] surface normal, in the world coordinates
// pos		.. [in] surface position, in the world coordinates
// eye		.. [in] direction from the surface to the eye, in the world coordinates
// shi		.. [in] shininess of the material
void EvaluatePhongLight(in PhongLight light, out vec3 amb, out vec3 dif, out vec3 spe, in vec3 normal, in vec3 position, in vec3 eye, in float shininess)
{
	vec3 L_not_normalized = light.position.xyz - pos * light.position.w;
	vec3 L = normalize(L_not_normalized);
	vec3 H = normalize(L + eye);

	// Calculate basic Phong factors
	float Iamb = 1.0;
	float Idif = max(dot(norm, L), 0.0);
	float Ispe = (Idif > 0.0) ? pow(max(dot(norm, H), 0.0), shi) : 0.0;

	// Calculate spot light factor
	if (light.spot_cos_cutoff != -1.0)
	{
		// This is a spot light
		float spot_factor;

		float spot_cos_angle = dot(-L, light.spot_direction);
		if (spot_cos_angle > light.spot_cos_cutoff)
		{
			spot_factor = pow(spot_cos_angle, light.spot_exponent);
		}
		else spot_factor = 0.0;

		Iamb *= 1.0;
		Idif *= spot_factor;
		Ispe *= spot_factor;
	}

	// Calculate attenuation
	if (light.position.w != 0.0)
	{
		// This is a point light / spot light

		float distance_from_light = length(L_not_normalized);
		float atten_factor =
			light.atten_constant +
			light.atten_linear * distance_from_light + 
			light.atten_quadratic * distance_from_light * distance_from_light;
		atten_factor = 1.0 / atten_factor;

		Iamb *= atten_factor;
		Idif *= atten_factor;
		Ispe *= atten_factor;
	}

	amb = Iamb * light.ambient;
	dif = Idif * light.diffuse;
	spe = Ispe * light.specular;
}

	*/

	//-----------------------------------
	//----    PHONG MATERIAL DATA    ----
	//-----------------------------------

	/// Basic structure which contains the data of a Phong material. It uses the same layout as
	/// its GLSL counterpart.
	struct PhongMaterial
	{
		/// Ambient part of the material
		glm::vec3 ambient;			float padding1;
		/// Diffuse part of the material
		glm::vec3 diffuse;
		/// Alpha (transparency) of the material
		float alpha;
		/// Specular part of the material
		glm::vec3 specular;
		/// Shininess of the material
		float shininess;

		static PhongMaterial CreateMaterial(
			const glm::vec3 &ambient,
			const glm::vec3 &diffuse,
			const glm::vec3 &specular,
			float shininess,
			float alpha);

		/// Basic colors. Sets ambient and diffuse to the same basic color. Specular is black
		/// if shininess is zero, white if white_specular is true, or the basic color otherwise.
		static PhongMaterial CreateBasicMaterial(const glm::vec3 &color, bool white_specular, float shininess, float alpha);
	};

	class MaterialData_UBO
	{
	private:
		std::vector<PhongMaterial> data;
		GLuint buffer;
		GLenum target;

	public:
		// Common methods, see the comment at the beginning of this file for more info.
		MaterialData_UBO();
		void Destroy();
		GLuint GetBuffer();
		void BindBuffer(GLuint index);
		void UpdateOpenGLData();

		/// Creates all OpenGL objects, allocates the UBO to be able to contain given number of materials.
		void Init(size_t count = 1, GLenum target = GL_UNIFORM_BUFFER);

		/// Sets the data of the material
		void SetMaterial(const PhongMaterial &material);
		/// Sets the data of of a given material
		void SetMaterial(int idx, const PhongMaterial &material);
	};

	/* Use this code in shaders

	- single material

layout (std140) uniform MaterialData
{
	// See the C++ code for the documentation to individual attributes
	vec3 ambient;
	vec3 diffuse;
	float alpha;
	vec3 specular;
	float shininess;
} material;

	- or a set of multiple materials

struct PhongMaterial
{
	// See the C++ code for the documentation to individual attributes
	vec3 ambient;
	vec3 diffuse;
	float alpha;
	vec3 specular;
	float shininess;
};
layout (std140) uniform MaterialData
{
	PhongMaterial materials[#count];
};

	*/

}

#endif	// INCLUDED_PV227_UBOS_H