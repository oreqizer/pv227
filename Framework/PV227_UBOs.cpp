#include "PV227.h"

#include <algorithm>

using namespace std;

namespace PV227
{

	//---------------------------
	//----    CAMERA DATA    ----
	//---------------------------

	CameraData_UBO::CameraData_UBO(): buffer(0), target(GL_UNIFORM_BUFFER)
	{
		// Assert the layout
		static_assert(offsetof(CameraData_UBO::SingleCameraData, projection) == 0,		"Incorrect CameraData layout");
		static_assert(offsetof(CameraData_UBO::SingleCameraData, projection_inv) == 64,	"Incorrect CameraData layout");
		static_assert(offsetof(CameraData_UBO::SingleCameraData, view) == 128,			"Incorrect CameraData layout");
		static_assert(offsetof(CameraData_UBO::SingleCameraData, view_inv) == 192,		"Incorrect CameraData layout");
		static_assert(offsetof(CameraData_UBO::SingleCameraData, view_it) == 256,		"Incorrect CameraData layout");
		static_assert(offsetof(CameraData_UBO::SingleCameraData, eye_position) == 304,	"Incorrect CameraData layout");
		static_assert(sizeof(CameraData_UBO::SingleCameraData) == 320,					"Incorrect CameraData layout");
	}

	void CameraData_UBO::Destroy()
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
		data.clear();
	}

	GLuint CameraData_UBO::GetBuffer() const
	{
		return buffer;
	}

	void CameraData_UBO::BindBuffer(GLuint index) const
	{
		glBindBufferBase(target, index, buffer);
	}

	void CameraData_UBO::UpdateOpenGLData() const
	{
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferSubData(target, 0, sizeof(SingleCameraData) * data.size(), data.data());
		glBindBuffer(target, 0);
	}

	void CameraData_UBO::Init(size_t count, GLenum target)
	{
		Destroy();
		this->target = target;
		
		data.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			data[i].projection = glm::mat4(1.0f);
			data[i].projection_inv = glm::mat4(1.0f);
			data[i].view = glm::mat4(1.0f);
			data[i].view_inv = glm::mat4(1.0f);
			data[i].view_it = glm::mat3x4(glm::mat3(1.0f));
			data[i].eye_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		glGenBuffers(1, &buffer);
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferData(target, sizeof(SingleCameraData) * count, data.data(), GL_STATIC_DRAW);
		glBindBuffer(target, 0);
	}

	void CameraData_UBO::SetProjection(const glm::mat4 &projection_matrix)
	{
		SetProjection(0, projection_matrix);
	}

	void CameraData_UBO::SetCamera(const SimpleCamera &camera)
	{
		SetCamera(0, camera);
	}

	void CameraData_UBO::SetCamera(const glm::mat4 &view_matrix)
	{
		SetCamera(0, view_matrix);
	}

	void CameraData_UBO::SetProjection(int idx, const glm::mat4 &projection_matrix)
	{
		data[idx].projection = projection_matrix;
		data[idx].projection_inv = glm::inverse(projection_matrix);
	}

	void CameraData_UBO::SetCamera(int idx, const SimpleCamera &camera)
	{
		data[idx].view = camera.GetViewMatrix();
		data[idx].view_inv = camera.GetViewInvMatrix();
		data[idx].view_it = glm::mat3x4(glm::transpose(glm::inverse(glm::mat3(data[idx].view))));
		data[idx].eye_position = glm::vec4(camera.GetEyePosition(), 1.0f);
	}

	void CameraData_UBO::SetCamera(int idx, const glm::mat4 &view_matrix)
	{
		data[idx].view = view_matrix;
		data[idx].view_inv = glm::inverse(view_matrix);
		data[idx].view_it = glm::mat3x4(glm::transpose(glm::inverse(glm::mat3(data[idx].view))));
		data[idx].eye_position = glm::vec4(glm::vec3(data[idx].view_inv[3]), 1.0f);
	}

	//--------------------------
	//----    MODEL DATA    ----
	//--------------------------

	ModelData_UBO::ModelData_UBO(): buffer(0), target(GL_UNIFORM_BUFFER)
	{
		// Assert the layout
		static_assert(offsetof(ModelData_UBO::SingleModelData, model) == 0,			"Incorrect ModelData layout");
		static_assert(offsetof(ModelData_UBO::SingleModelData, model_inv) == 64,	"Incorrect ModelData layout");
		static_assert(offsetof(ModelData_UBO::SingleModelData, model_it) == 128,	"Incorrect ModelData layout");
		static_assert(sizeof(ModelData_UBO::SingleModelData) == 176,				"Incorrect ModelData layout");
	}

	void ModelData_UBO::Destroy()
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}

	GLuint ModelData_UBO::GetBuffer()
	{
		return buffer;
	}

	void ModelData_UBO::BindBuffer(GLuint index)
	{
		glBindBufferBase(target, index, buffer);
	}

	void ModelData_UBO::UpdateOpenGLData()
	{
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferSubData(target, 0, sizeof(SingleModelData) * data.size(), data.data());
		glBindBuffer(target, 0);
	}

	void ModelData_UBO::Init(size_t count, GLenum target)
	{
		Destroy();
		this->target = target;

		data.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			data[i].model = glm::mat4(1.0f);
			data[i].model_inv = glm::mat4(1.0f);
			data[i].model_it = glm::mat3x4(1.0f);
		}

		glGenBuffers(1, &buffer);
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferData(target, sizeof(SingleModelData) * count, data.data(), GL_STATIC_DRAW);
		glBindBuffer(target, 0);
	}

	void ModelData_UBO::SetMatrix(const glm::mat4 &model)
	{
		SetMatrix(0, model);
	}

	void ModelData_UBO::SetMatrix(int idx, const glm::mat4 &model)
	{
		data[idx].model = model;
		data[idx].model_inv = glm::inverse(model);
		data[idx].model_it = glm::mat3x4(glm::transpose(glm::inverse(glm::mat3(data[idx].model))));
	}

	//---------------------------------
	//----    PHONG LIGHTS DATA    ----
	//---------------------------------

	// Assert the layout
	static_assert(offsetof(PhongLight, position) == 0,			"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, ambient) == 16,			"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, diffuse) == 32,			"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, specular) == 48,			"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, spot_direction) == 64,	"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, spot_exponent) == 76,	"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, spot_cos_cutoff) == 80,	"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, atten_constant) == 84,	"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, atten_linear) == 88,		"Incorrect PhongLight layout");
	static_assert(offsetof(PhongLight, atten_quadratic) == 92,	"Incorrect PhongLight layout");
	static_assert(sizeof(PhongLight) == 96,						"Incorrect PhongLight layout");

	PhongLight PhongLight::CreateLight(
		const glm::vec4 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular,
		const glm::vec3 &spot_direction,
		float spot_exponent,
		float spot_cos_cutoff,
		float atten_constant,
		float atten_linear,
		float atten_quadratic)
	{
		PhongLight light;
		light.position = position;
		light.ambient = ambient;
		light.diffuse = diffuse;
		light.specular = specular;
		light.spot_direction = spot_direction;
		light.spot_exponent = spot_exponent;
		light.spot_cos_cutoff = spot_cos_cutoff;
		light.atten_constant = atten_constant;
		light.atten_linear = atten_linear;
		light.atten_quadratic = atten_quadratic;
		return light;
	}

	PhongLight PhongLight::CreatePointLight(
		const glm::vec3 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular)
	{
		return CreatePointLight(position, ambient, diffuse, specular, 1.0f, 0.0f, 0.0f);
	}

	PhongLight PhongLight::CreatePointLight(
		const glm::vec3 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular,
		float atten_constant,
		float atten_linear,
		float atten_quadratic)
	{
		return CreateLight(glm::vec4(position, 1.0f), ambient, diffuse, specular,
			glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, -1.0f, atten_constant, atten_linear, atten_quadratic);
	}

	PhongLight PhongLight::CreateDirectionalLight(
		const glm::vec3 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular)
	{
		return CreateLight(glm::vec4(position, 0.0f), ambient, diffuse, specular,
			glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, -1.0f, 1.0f, 0.0f, 0.0f);
	}

	PhongLight PhongLight::CreateSpotLight(
		const glm::vec3 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular,
		const glm::vec3 &spot_direction,
		float spot_exponent,
		float spot_cos_cutoff)
	{
		return CreateSpotLight(position, ambient, diffuse, specular, spot_direction, spot_exponent, spot_cos_cutoff, 1.0f, 0.0f, 0.0f);
	}

	PhongLight PhongLight::CreateSpotLight(
		const glm::vec3 &position,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular,
		const glm::vec3 &spot_direction,
		float spot_exponent,
		float spot_cos_cutoff,
		float atten_constant,
		float atten_linear,
		float atten_quadratic)
	{
		return CreateLight(glm::vec4(position, 1.0f), ambient, diffuse, specular,
			spot_direction, spot_exponent, spot_cos_cutoff, atten_constant, atten_linear, atten_quadratic);
	}

	PhongLight PhongLight::CreateHeadLight(
		const glm::mat4 &view_matrix,
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular)
	{
		// Head light is a point light with its position at the position of the eye.
		// The position of the eye is the translation part (the last column) of the inverse of the view matrix.
		return CreatePointLight(glm::vec3(glm::inverse(view_matrix)[3]), ambient, diffuse, specular);
	}

	//--  PhongLightsData_UBO

	PhongLightsData_UBO::PhongLightsData_UBO(): buffer(0), max_lights(0), target(GL_UNIFORM_BUFFER)
	{
		header.global_ambient_color = glm::vec3(0.0f);
	}

	void PhongLightsData_UBO::Destroy()
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}

	GLuint PhongLightsData_UBO::GetBuffer()
	{
		return buffer;
	}

	void PhongLightsData_UBO::BindBuffer(GLuint index)
	{
		glBindBufferBase(target, index, buffer);
	}

	void PhongLightsData_UBO::UpdateOpenGLData()
	{
		header.lights_count = std::min(int(max_lights), int(PhongLights.size()));
		
		glBindBuffer(target, buffer);
		glBufferSubData(target, 0, sizeof(PhongLightsDataHeader), &header);
		if (header.lights_count > 0)
			glBufferSubData(target, sizeof(PhongLightsDataHeader), sizeof(PhongLight) * header.lights_count, PhongLights.data());
		glBindBuffer(target, 0);
	}

	void PhongLightsData_UBO::Init(size_t max_lights, GLenum target)
	{
		Destroy();
		this->max_lights = max_lights;
		this->target = target;

		glGenBuffers(1, &buffer);
		glBindBuffer(this->target, buffer);
		glBufferData(this->target, sizeof(PhongLightsDataHeader) + sizeof(PhongLight) * this->max_lights, nullptr, GL_STATIC_DRAW);
		glBindBuffer(this->target, 0);
	}

	void PhongLightsData_UBO::SetGlobalAmbient(const glm::vec3 &global_ambient_color)
	{
		header.global_ambient_color = global_ambient_color;
	}

	//-----------------------------------
	//----    PHONG MATERIAL DATA    ----
	//-----------------------------------

	PhongMaterial PhongMaterial::CreateMaterial(
		const glm::vec3 &ambient,
		const glm::vec3 &diffuse,
		const glm::vec3 &specular,
		float shininess,
		float alpha)
	{
		PhongMaterial material;
		material.ambient = ambient;
		material.diffuse = diffuse;
		material.specular = specular;
		material.shininess = shininess;
		material.alpha = alpha;
		return material;
	}
	PhongMaterial PhongMaterial::CreateBasicMaterial(const glm::vec3 &color, bool white_specular, float shininess, float alpha)
	{
		return CreateMaterial(color, color, (shininess == 0.0f) ? glm::vec3(0.0f) : (white_specular ? glm::vec3(1.0f) : color), shininess, alpha);
	}

	MaterialData_UBO::MaterialData_UBO(): buffer(0), target(GL_UNIFORM_BUFFER)
	{
	}
	
	void MaterialData_UBO::Destroy()
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}

	GLuint MaterialData_UBO::GetBuffer()
	{
		return buffer;
	}
	
	void MaterialData_UBO::BindBuffer(GLuint index)
	{
		glBindBufferBase(target, index, buffer);
	}

	void MaterialData_UBO::UpdateOpenGLData()
	{
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferSubData(target, 0, sizeof(PhongMaterial) * data.size(), data.data());
		glBindBuffer(target, 0);
	}

	void MaterialData_UBO::Init(size_t count, GLenum target)
	{
		Destroy();
		this->target = target;
	
		data.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			data[i] = PhongMaterial::CreateMaterial(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), 0.0f, 1.0f);
		}

		glGenBuffers(1, &buffer);
		glBindBuffer(target, buffer);
		if (!data.empty())
			glBufferData(target, sizeof(PhongMaterial) * count, data.data(), GL_STATIC_DRAW);
		glBindBuffer(target, 0);
	}

	void MaterialData_UBO::SetMaterial(const PhongMaterial &material)
	{
		SetMaterial(0, material);
	}

	void MaterialData_UBO::SetMaterial(int idx, const PhongMaterial &material)
	{
		data[idx] = material;
	}

}