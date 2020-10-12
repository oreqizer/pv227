#version 430 core

// Input variables
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

// Output variables
out VertexData
{
	vec3 normal_ws;		// Normal in world space
	vec3 position_ws;	// Position in world space
	vec2 tex_coord;
} outData;

// Data of the camera
layout (std140, binding = 0) uniform CameraData
{
	mat4 projection;		// Projection matrix
	mat4 projection_inv;	// Inverse of the projection matrix
	mat4 view;				// View matrix
	mat4 view_inv;			// Inverse of the view matrix
	mat3 view_it;			// Inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// Position of the eye in world space
};

// Data of the object
layout (std140, binding = 2) uniform ModelData
{
	mat4 model;			// Model matrix
	mat4 model_inv;		// Inverse of the model matrix
	mat3 model_it;		// Inverse of the transpose of the top-left part 3x3 of the model matrix
};

//-----------------------------------------------------------------------

void main()
{
	outData.tex_coord = tex_coord;
	outData.position_ws = vec3(model * position);
	outData.normal_ws = normalize(model_it * normal);

	gl_Position = projection * view * model * position;
}
