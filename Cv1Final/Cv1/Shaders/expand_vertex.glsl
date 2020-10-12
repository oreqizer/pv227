#version 430 core

// Input variables
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

// Output variables - no output variables

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
	// Solution #1 (prefered) using normal of the object 
	// - problems can occur for nonconvex objects if the shift would be too big 
	// - works well for (convex) objects with holes (e.g., torus)
	vec4 pos = position + vec4(normal, 0.0) * 0.02;
	gl_Position = projection * view * model * pos;

	// Solution #2 using the scaling from the center of the object 
	// - problems can occur for objects placed outside (0,0,0) in their local coordinate system
	// - does not work well for objects with holes (e.g., torus)
	//float scale = 1.02;
	//gl_Position = projection * view * model * (vec4(scale,scale,scale,1) * position);
}
