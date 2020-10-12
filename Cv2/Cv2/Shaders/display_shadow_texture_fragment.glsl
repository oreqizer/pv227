#version 430 core

layout (location = 0) out vec4 final_color;

in VertexData
{
	vec2 tex_coord;
} inData;

layout (binding = 0) uniform sampler2D shadow_tex;

void main()
{
	// Task 2: Display the shadow map
	final_color = vec4(texture(shadow_tex, inData.tex_coord).rrr, 1.0);
}
