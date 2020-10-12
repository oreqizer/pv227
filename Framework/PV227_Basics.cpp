#include "PV227_Basics.h"

#pragma comment(lib, "glew32s.lib")			// Link with GLEW library
#pragma comment(lib, "DevIL.lib")			// Link with DevIL library
#pragma comment(lib, "AntTweakBar.lib")		// Link with AntTweakBar library

// Asks for the context to be created on nVidia graphics when optimus is enabled
// Credits: Martin Matous, 410309
extern "C"
{
#if defined(_WIN32)
	__declspec(dllexport) int NvOptimusEnablement = 0x00000001;
#else
	// TODO Version for Linux/Mac (not tested)
	int NvOptimusEnablement = 0x00000001;
#endif
}

#if defined(_WIN32)
#define NOMINMAX				// Make Windows.h not define 'min' and 'max' macros
#include <GL/wglew.h>			// Include on Windows
#else
#include <GL/glxew.h>			// Include on Linux and Mac
#endif

// Include DevIL for image loading
#ifndef _UNICODE
	// We have UNICODE version of DevIL dll. We must define _UNICODE
	// when including il.h even when it is not defined
	#define _UNICODE
	#include <IL/il.h>
	#undef _UNICODE
#else
	#include <IL/il.h>
#endif

#include <memory>
#include <sstream>
#include <fstream>

#include "../inlines/tangentcube.inl"
#include "../inlines/tangentsphere.inl"
#include "../inlines/tangenttorus.inl"
#include "../inlines/tangentcylinder.inl"
#include "../inlines/tangentcapsule.inl"
#include "../inlines/tangentteapot.inl"
#include "../inlines/tangentteapotpatch.inl"

using namespace std;

namespace PV227
{

	//---------------------------
	//----    APPLICATION    ----
	//---------------------------

	void PrintOpenGLInfo()
	{
		cout << "Vendor   : " << (const char *)glGetString(GL_VENDOR) << endl;
		cout << "Renderer : " << (const char *)glGetString(GL_RENDERER) << endl;
		cout << "Version  : " << (const char *)glGetString(GL_VERSION) << endl;
		cout << endl;
	}

	bool IsOpenGLExtensionPresent(const char *extension)
	{
		// Due to a bug in GLEW, we cannot use glewGetExtension, so we must test for the extension by ourselves.

		// Get the number of extensions
		GLint num_extension;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_extension);
	
		// Go through all extensions and check whether the given extension is present
		for (int i = 0; i < num_extension; i++)
		{
			if (strcmp((const char *)glGetStringi(GL_EXTENSIONS, i), extension) == 0)
				return true;		// Extension is found
		}
		return false;		// Extension is not found
	}

	void SetDebugCallback(GLDEBUGPROCARB callback)
	{
		// Setup callback that will inform us when we make an error.
		// glDebugMessageCallbackARB is sometimes missed by glew, due to a bug in it.

#if defined(_WIN32)
		// On Windows, use this:
		PFNGLDEBUGMESSAGECALLBACKARBPROC myglDebugMessageCallbackARB =
			(PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
		if (myglDebugMessageCallbackARB)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			myglDebugMessageCallbackARB(callback, nullptr);
		}
#elif defined(__APPLE__)
		// On MacOS, use this (not tested):
		if (glDebugMessageCallbackARB)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallbackARB(callback, nullptr);
		}
#else
		// On Linux, use this (not tested):
		PFNGLDEBUGMESSAGECALLBACKARBPROC myglDebugMessageCallbackARB =
			(PFNGLDEBUGMESSAGECALLBACKARBPROC)glXGetProcAddress((unsigned char *)"glDebugMessageCallbackARB");
		if (myglDebugMessageCallbackARB)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			myglDebugMessageCallbackARB(callback, nullptr);
		}
#endif
	}

	void SetVSync(bool use_v_sync)
	{
#if defined(_WIN32)
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(use_v_sync ? 1 : 0);
#else
		// According to the example 1 of https://www.opengl.org/registry/specs/EXT/swap_control.txt
		if (glXSwapIntervalEXT)
		{
			Display *dpy = glXGetCurrentDisplay();
			GLXDrawable drawable = glXGetCurrentDrawable();
			if (drawable)
			{
				glXSwapIntervalEXT(dpy, drawable, use_v_sync ? 1 : 0);
			}
		}
#endif
	}

	float GetFPS()
	{
#if defined(_WIN32)
		static int64_t last_tick_count = 0;
		LARGE_INTEGER current_tick_count, frequency;
		QueryPerformanceCounter(&current_tick_count);
		QueryPerformanceFrequency(&frequency);
		int64_t tick_diff = int64_t(current_tick_count.QuadPart) - last_tick_count;
		last_tick_count = int64_t(current_tick_count.QuadPart);
		double diff_s = double(tick_diff) / double(frequency.QuadPart);
		return 1.0f / float(diff_s);
#else
		return -1.0f;		// I'm sorry, I don't have Linux equivalent yet
#endif
	}

	//------------------------------------
	//----    SHADERS AND PROGRAMS    ----
	//------------------------------------

	string LoadFileToString(const char *file_name)
	{
		ifstream file(file_name);
		stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	GLuint LoadAndCompileShader(GLenum shader_type, const char *file_name)
	{
		// Load the file from the disk
		string s_source = LoadFileToString(file_name);
		if (s_source.empty())
		{
			cout << "File " << file_name << " is empty or failed to load" << endl;
			return 0;
		}

		// Create shader object and set the source
		GLuint shader = glCreateShader(shader_type);
		const char *source = s_source.c_str();
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		// Compile and get errors
		int compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (GL_FALSE == compile_status)
		{
			switch (shader_type)
			{
			case GL_VERTEX_SHADER:			cout << "Failed to compile vertex shader " << file_name << endl;					break;
			case GL_FRAGMENT_SHADER:		cout << "Failed to compile fragment shader " << file_name << endl;					break;
			case GL_GEOMETRY_SHADER:		cout << "Failed to compile geometry shader " << file_name << endl;					break;
			case GL_TESS_CONTROL_SHADER:	cout << "Failed to compile tessellation control shader " << file_name << endl;		break;
			case GL_TESS_EVALUATION_SHADER:	cout << "Failed to compile tessellation evaluation shader " << file_name << endl;	break;
			case GL_COMPUTE_SHADER:			cout << "Failed to compile compute shader " << file_name << endl;					break;
			default:						cout << "Failed to compile shader " << file_name << endl;							break;
			}
			
			int log_len = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
			unique_ptr<char []> log(new char[log_len]);
			glGetShaderInfoLog(shader, log_len, nullptr, log.get());
			cout << log.get() << endl;

			glDeleteShader(shader);
			return 0;
		}
		else return shader;
	}

	ShaderProgram::ShaderProgram(): program(0), valid(false)
	{
	}

	void ShaderProgram::Init()
	{
		Destroy();
		program = glCreateProgram();
	}

	void ShaderProgram::Destroy()
	{
		valid = false;

		glDeleteProgram(program);
		program = 0;
		
		for (Shader &shader : shaders)
			glDeleteShader(shader.shader);
		shaders.clear();
	}

	bool ShaderProgram::AddShader(GLenum shader_type, const char *file_name)
	{
		if (!program)
			return false;		// Ignore silently

		// Load and compile the shader
		Shader shader;
		shader.shader = LoadAndCompileShader(shader_type, file_name);
		shader.type = shader_type;
		shader.file_name = file_name;
		shaders.push_back(shader);
		
		if (shader.shader)
		{
			glAttachShader(program, shader.shader);
			return true;
		}
		else
		{
			Destroy();
			return false;
		}
	}

	bool ShaderProgram::AddVertexShader(const char *file_name)
	{
		return AddShader(GL_VERTEX_SHADER, file_name);
	}

	bool ShaderProgram::AddTessControlShader(const char *file_name)
	{
		return AddShader(GL_TESS_CONTROL_SHADER, file_name);
	}

	bool ShaderProgram::AddTessEvaluationShader(const char *file_name)
	{
		return AddShader(GL_TESS_EVALUATION_SHADER, file_name);
	}

	bool ShaderProgram::AddGeometryShader(const char *file_name)
	{
		return AddShader(GL_GEOMETRY_SHADER, file_name);
	}

	bool ShaderProgram::AddFragmentShader(const char *file_name)
	{
		return AddShader(GL_FRAGMENT_SHADER, file_name);
	}

	bool ShaderProgram::AddComputeShader(const char *file_name)
	{
		return AddShader(GL_COMPUTE_SHADER, file_name);
	}

	void ShaderProgram::BindAttribLocation(GLint idx, const char *name) const
	{
		if (program)			// Ignore silently
			glBindAttribLocation(program, idx, name);
	}

	void ShaderProgram::BindFragDataLocation(GLint idx, const char *name) const
	{
		if (program)			// Ignore silently
			glBindFragDataLocation(program, idx, name);
	}

	bool ShaderProgram::Link()
	{
		if (!program)
			return false;		// Ignore silently

		// Link program
		glLinkProgram(program);

		// Link and get errors
		int link_status;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		if (GL_FALSE == link_status)
		{
			cout << "Failed to link program with shaders: ";
			for (Shader &shader : shaders)
				cout << shader.file_name << ", ";
			cout << endl;
			
			int log_len = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
			unique_ptr<char []> log(new char[log_len]);
			glGetProgramInfoLog(program, log_len, nullptr, log.get());
			cout << log.get() << endl;

			Destroy();
			return false;
		}
		else
		{
			valid = true;
			return true;
		}
	}

	bool ShaderProgram::IsValid() const
	{
		return valid;
	}

	GLuint ShaderProgram::GetProgram() const
	{
		return program;
	}

	GLint ShaderProgram::GetAttribLocation(const char *name) const
	{
		if (valid)
			return glGetAttribLocation(program, name);
		else return -1;
	}

	GLint ShaderProgram::GetUniformLocation(const char *name) const
	{
		if (valid)
			return glGetUniformLocation(program, name);
		else return -1;
	}

	GLuint ShaderProgram::GetUniformBlockIndex(const char *name) const
	{
		if (valid)
			return glGetUniformBlockIndex(program, name);
		else return GL_INVALID_INDEX;
	}

	void ShaderProgram::SetUniformBlockBindingIndex(const char *name, GLuint idx) const
	{
		if (valid)
		{
			GLuint loc = glGetUniformBlockIndex(program, name);
			if (loc != GL_INVALID_INDEX)
				glUniformBlockBinding(program, loc, idx);
		}
	}

	void ShaderProgram::Use() const
	{
		glUseProgram(program);
	}

	void ShaderProgram::Uniform1f			(const char *name, GLfloat v0) const												{	glUniform1f		(GetUniformLocation(name), v0);					}
	void ShaderProgram::Uniform1fv			(const char *name, GLsizei count, const GLfloat* value) const						{	glUniform1fv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform1i			(const char *name, GLint v0) const													{	glUniform1i		(GetUniformLocation(name), v0);					}
	void ShaderProgram::Uniform1iv			(const char *name, GLsizei count, const GLint* value) const							{	glUniform1iv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform2f			(const char *name, GLfloat v0, GLfloat v1) const									{	glUniform2f		(GetUniformLocation(name), v0, v1);				}
	void ShaderProgram::Uniform2fv			(const char *name, GLsizei count, const GLfloat* value) const						{	glUniform2fv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform2i			(const char *name, GLint v0, GLint v1) const										{	glUniform2i		(GetUniformLocation(name), v0, v1);				}
	void ShaderProgram::Uniform2iv			(const char *name, GLsizei count, const GLint* value) const							{	glUniform2iv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform3f			(const char *name, GLfloat v0, GLfloat v1, GLfloat v2) const						{	glUniform3f		(GetUniformLocation(name), v0, v1, v2);			}
	void ShaderProgram::Uniform3fv			(const char *name, GLsizei count, const GLfloat* value) const						{	glUniform3fv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform3i			(const char *name, GLint v0, GLint v1, GLint v2) const								{	glUniform3i		(GetUniformLocation(name), v0, v1, v2);			}
	void ShaderProgram::Uniform3iv			(const char *name, GLsizei count, const GLint* value) const							{	glUniform3iv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform4f			(const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const			{	glUniform4f		(GetUniformLocation(name), v0, v1, v2, v3);		}
	void ShaderProgram::Uniform4fv			(const char *name, GLsizei count, const GLfloat* value) const						{	glUniform4fv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::Uniform4i			(const char *name, GLint v0, GLint v1, GLint v2, GLint v3) const					{	glUniform4i		(GetUniformLocation(name), v0, v1, v2, v3);		}
	void ShaderProgram::Uniform4iv			(const char *name, GLsizei count, const GLint* value) const							{	glUniform4iv	(GetUniformLocation(name), count, value);		}
	void ShaderProgram::UniformMatrix2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix2fv		(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix3fv		(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix4fv		(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix2x3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix2x3fv	(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix2x4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix2x4fv	(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix3x2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix3x2fv	(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix3x4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix3x4fv	(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix4x2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix4x2fv	(GetUniformLocation(name), count, transpose, value);	}
	void ShaderProgram::UniformMatrix4x3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const	{	glUniformMatrix4x3fv	(GetUniformLocation(name), count, transpose, value);	}

	void ShaderProgramMap::AddProgram(int key, ShaderProgram *program)
	{
		if (!program)
			RemoveProgram(key);
		else
			shader_map[key] = program;
	}

	void ShaderProgramMap::RemoveProgram(int key)
	{
		shader_map.erase(key);
	}

	void ShaderProgramMap::RemoveAll()
	{
		shader_map.clear();
	}

	ShaderProgram *ShaderProgramMap::GetProgram(int key)
	{
		auto iter = shader_map.find(key);
		return (iter == shader_map.end()) ? nullptr : iter->second;
	}

	bool ShaderProgramMap::UseProgram(int key)
	{
		ShaderProgram *program = GetProgram(key);
		if (!program || !program->IsValid())
		{
			return false;
		}
		else
		{
			program->Use();
			return true;
		}
	}

	//------------------------------
	//----    GEOMETRY CLASS    ----
	//------------------------------

	Geometry::Geometry()
	{
		IndexBuffer = 0;
		VAO = 0;
		Mode = GL_POINTS;
		DrawArraysCount = 0;
		DrawElementsCount = 0;
		PatchVertices = 0;
	}

	Geometry::Geometry(const Geometry &rhs)
	{
		*this = rhs;
	}

	Geometry &Geometry::operator =(const Geometry &rhs)
	{
		VertexBuffers = rhs.VertexBuffers;
		IndexBuffer = rhs.IndexBuffer;
		VAO = rhs.VAO;
		Mode = rhs.Mode;
		DrawArraysCount = rhs.DrawArraysCount;
		DrawElementsCount = rhs.DrawElementsCount;
		PatchVertices = rhs.PatchVertices;
		return *this;
	}

	void Geometry::Destroy()
	{
		// When using it, make sure the OpenGL context still exists (i.e. the main window still exists).
		
		// OpenGL silently ignores deleting objects that are 0, so this is safe even if the buffers were not created.
		if (!VertexBuffers.empty())
			glDeleteBuffers(VertexBuffers.size(), VertexBuffers.data());
		glDeleteBuffers(1, &IndexBuffer);
		glDeleteVertexArrays(1, &VAO);

		*this = Geometry();		// Reset the state to 'no geometry'
	}

	void Geometry::BindVAO() const
	{
		glBindVertexArray(VAO);
	}

	void Geometry::Draw() const
	{
		if (Mode == GL_PATCHES)
			glPatchParameteri(GL_PATCH_VERTICES, PatchVertices);
		if (DrawArraysCount > 0)
			glDrawArrays(Mode, 0, DrawArraysCount);
		if (DrawElementsCount > 0)
			glDrawElements(Mode, DrawElementsCount, GL_UNSIGNED_INT, nullptr);
	}

	void Geometry::DrawInstanced(int primcount) const
	{
		if (Mode == GL_PATCHES)
			glPatchParameteri(GL_PATCH_VERTICES, PatchVertices);
		if (DrawArraysCount > 0)
			glDrawArraysInstanced(Mode, 0, DrawArraysCount, primcount);
		if (DrawElementsCount > 0)
			glDrawElementsInstanced(Mode, DrawElementsCount, GL_UNSIGNED_INT, nullptr, primcount);
	}

	Geometry CreateCube(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentcube_vertices_count * sizeof(float) * 14, tangentcube_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangentcube_indices_count * sizeof(unsigned int), tangentcube_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLES;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangentcube_indices_count;

		return geometry;
	}

	Geometry CreateSphere(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentsphere_vertices_count * sizeof(float) * 14, tangentsphere_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangentsphere_indices_count * sizeof(unsigned int), tangentsphere_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLE_STRIP;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangentsphere_indices_count;

		return geometry;
	}

	Geometry CreateTorus(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangenttorus_vertices_count * sizeof(float) * 14, tangenttorus_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangenttorus_indices_count * sizeof(unsigned int), tangenttorus_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLE_STRIP;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangenttorus_indices_count;

		return geometry;
	}

	Geometry CreateCylinder(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentcylinder_vertices_count * sizeof(float) * 14, tangentcylinder_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangentcylinder_indices_count * sizeof(unsigned int), tangentcylinder_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLE_STRIP;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangentcylinder_indices_count;

		return geometry;
	}

	Geometry CreateCapsule(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentcapsule_vertices_count * sizeof(float) * 14, tangentcapsule_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangentcapsule_indices_count * sizeof(unsigned int), tangentcapsule_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLE_STRIP;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangentcapsule_indices_count;

		return geometry;
	}

	Geometry CreateTeapot(GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentteapot_vertices_count * sizeof(float) * 14, tangentteapot_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Create a buffer for indices
		glGenBuffers(1, &geometry.IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangentteapot_indices_count * sizeof(unsigned int), tangentteapot_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, 0);
		}
		if (normal_loc >= 0)
		{
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 3));
		}
		if (tex_coord_loc >= 0)
		{
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 6));
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 8));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (const void *)(sizeof(float) * 11));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.IndexBuffer);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLE_STRIP;
		geometry.DrawArraysCount = 0;
		geometry.DrawElementsCount = tangentteapot_indices_count;

		return geometry;
	}

	Geometry CreateTeapotPatch(GLint position_loc, GLint tangent_loc, GLint bitangent_loc)
	{
		Geometry geometry;

		// Create a single buffer for vertex data
		geometry.VertexBuffers.resize(1, 0);
		glGenBuffers(1, &geometry.VertexBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, tangentteapotpatch_vertices_count * sizeof(float) * 9, tangentteapotpatch_vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// No index buffer

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		if (position_loc >= 0)
		{
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
		}
		if (tangent_loc >= 0)
		{
			glEnableVertexAttribArray(tangent_loc);
			glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (const void *)(sizeof(float) * 3));
		}
		if (bitangent_loc >= 0)
		{
			glEnableVertexAttribArray(bitangent_loc);
			glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (const void *)(sizeof(float) * 6));
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_PATCHES;
		geometry.PatchVertices = 16;
		geometry.DrawArraysCount = tangentteapotpatch_vertices_count;
		geometry.DrawElementsCount = 0;

		return geometry;
	}

	Geometry CreateFullscreenQuad()
	{
		Geometry geometry;

		// Use no buffers
		geometry.VertexBuffers.clear();
		geometry.IndexBuffer = 0;

		// Create an empty VAO
		glGenVertexArrays(1, &geometry.VAO);
		glBindVertexArray(geometry.VAO);
		glBindVertexArray(0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLES;
		geometry.DrawArraysCount = 3;
		geometry.DrawElementsCount = 0;

		return geometry;
	}

	/// Parses an OBJ file. For OBJ file format, see https://en.wikipedia.org/wiki/Wavefront_.obj_file
	///
	/// This OBJ parser is very simple and serves only for our lectures. It handles only geometries with
	/// triangles, and each vertex must have its position, normal, and texture coordinate defined. When
	/// parsing other OBJ files, write your own parser or download another one, you may use for example
	/// Open Asset Import Library (Assimp).
	///
	/// When the file is correctly parsed, the function returns true and 'out_vertices', 'out_normals' and
	/// 'out_tex_coords' contains the data of individual triangles (use glDrawArrays with GL_TRIANGLES).
	/// If something goes wrong, error messsage is printed and this function returns false.
	bool ParseOBJFile(const char *file_name, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec3> &out_normals, std::vector<glm::vec2> &out_tex_coords)
	{
		// I love lambda functions :-)
		auto error_msg = [file_name] {
			cout << "Failed to read OBJ file " << file_name << ", its format is not supported" << endl;
		};

		struct OBJTriangle
		{
			int v0, v1, v2;
			int n0, n1, n2;
			int t0, t1, t2;
		};

		// Prepare the arrays for the data from the file.
		std::vector<glm::vec3> raw_vertices;		raw_vertices.reserve(1000);
		std::vector<glm::vec3> raw_normals;			raw_normals.reserve(1000);
		std::vector<glm::vec2> raw_tex_coords;		raw_tex_coords.reserve(1000);
		std::vector<OBJTriangle> raw_triangles;		raw_triangles.reserve(1000);

		// Load OBJ file
		ifstream file(file_name);
		if (!file.is_open())
		{
			cout << "Cannot open OBJ file " << file_name << endl;
			return false;
		}

		while (!file.fail())
		{
			string prefix;
			file >> prefix;

			if (prefix == "v")
			{
				glm::vec3 v;
				file >> v.x >> v.y >> v.z;
				raw_vertices.push_back(v);
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
			else if (prefix == "vt")
			{
				glm::vec2 vt;
				file >> vt.x >> vt.y;
				raw_tex_coords.push_back(vt);
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
			else if (prefix == "vn")
			{
				glm::vec3 vn;
				file >> vn.x >> vn.y >> vn.z;
				raw_normals.push_back(vn);
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
			else if (prefix == "f")
			{
				OBJTriangle t;
				char slash;

				// And now check whether the geometry is of a correct format (that it contains only triangles,
				// and all vertices have their position, normal, and texture coordinate set).
				
				// Read the first vertex
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.v0;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.t0;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.n0;

				// Read the second vertex
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.v1;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.t1;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.n1;

				// Read the third vertex
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.v2;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.t2;
				file >> ws;		if (file.peek() != '/')		{	error_msg();		return false;	}
				file >> slash;
				file >> ws;		if (!isdigit(file.peek()))	{	error_msg();		return false;	}
				file >> t.n2;

				// Check that this polygon has only three vertices (we support triangles only).
				// It also skips all white spaces, effectively ignoring the rest of the line (if empty).
				file >> ws;		if (isdigit(file.peek()))	{	error_msg();		return false;	}

				// Subtract one, OBJ indexes from 1, not from 0
				t.v0--;		t.v1--;		t.v2--;
				t.n0--;		t.n1--;		t.n2--;
				t.t0--;		t.t1--;		t.t2--;

				raw_triangles.push_back(t);
			}
			else
			{
				// Ignore other cases
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
		}
		file.close();

		// Indices in OBJ file cannot be used, we need to convert the geometry in a way we could draw it
		// with glDrawArrays.
		out_vertices.clear();		out_vertices.reserve(raw_triangles.size() * 3);
		out_normals.clear();		out_normals.reserve(raw_triangles.size() * 3);
		out_tex_coords.clear();		out_tex_coords.reserve(raw_triangles.size() * 3);
		for (size_t i = 0; i < raw_triangles.size(); i++)
		{
			if ((raw_triangles[i].v0 >= int(raw_vertices.size())) ||
				(raw_triangles[i].v1 >= int(raw_vertices.size())) ||
				(raw_triangles[i].v2 >= int(raw_vertices.size())) ||
				(raw_triangles[i].n0 >= int(raw_normals.size())) ||
				(raw_triangles[i].n1 >= int(raw_normals.size())) ||
				(raw_triangles[i].n2 >= int(raw_normals.size())) ||
				(raw_triangles[i].t0 >= int(raw_tex_coords.size())) ||
				(raw_triangles[i].t1 >= int(raw_tex_coords.size())) ||
				(raw_triangles[i].t2 >= int(raw_tex_coords.size())))
			{
				// Invalid out-of-range indices
				error_msg();
				return false;
			}

			out_vertices.push_back(raw_vertices[raw_triangles[i].v0]);
			out_vertices.push_back(raw_vertices[raw_triangles[i].v1]);
			out_vertices.push_back(raw_vertices[raw_triangles[i].v2]);
			out_normals.push_back(raw_normals[raw_triangles[i].n0]);
			out_normals.push_back(raw_normals[raw_triangles[i].n1]);
			out_normals.push_back(raw_normals[raw_triangles[i].n2]);
			out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t0]);
			out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t1]);
			out_tex_coords.push_back(raw_tex_coords[raw_triangles[i].t2]);
		}

		return true;
	}

	Geometry LoadOBJ(const char *file_name, GLint position_loc, GLint normal_loc, GLint tex_coord_loc)
	{
		Geometry geometry;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> tex_coords;
		if (!ParseOBJFile(file_name, vertices, normals, tex_coords))
		{
			return geometry;		// Return empty geometry, the error message was already printed
		}

		// Create buffers for vertex data
		geometry.VertexBuffers.resize(3, 0);
		glGenBuffers(3, geometry.VertexBuffers.data());
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[1]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float) * 3, normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[2]);
		glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * sizeof(float) * 2, tex_coords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// No indices
		geometry.IndexBuffer = 0;

		// Create a vertex array object for the geometry
		glGenVertexArrays(1, &geometry.VAO);

		// Set the parameters of the geometry
		glBindVertexArray(geometry.VAO);
		if (position_loc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[0]);
			glEnableVertexAttribArray(position_loc);
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (normal_loc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[1]);
			glEnableVertexAttribArray(normal_loc);
			glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (tex_coord_loc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, geometry.VertexBuffers[2]);
			glEnableVertexAttribArray(tex_coord_loc);
			glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}
		
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Set the Mode and the number of vertices to draw
		geometry.Mode = GL_TRIANGLES;
		geometry.DrawArraysCount = vertices.size();
		geometry.DrawElementsCount = 0;

		return geometry;
	}

	bool LoadVerticesFromOBJFile(const char *file_name, std::vector<glm::vec3> &out_vertices)
	{
		// Prepare the arrays for the data from the file.
		std::vector<glm::vec3> raw_vertices;
		raw_vertices.reserve(1000);

		// Load OBJ file
		ifstream file(file_name);
		if (!file.is_open())
		{
			cout << "Cannot open OBJ file " << file_name << endl;
			return false;
		}

		while (!file.fail())
		{
			string prefix;
			file >> prefix;

			if (prefix == "v")
			{
				glm::vec3 v;
				file >> v.x >> v.y >> v.z;
				raw_vertices.push_back(v);
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
			else
			{
				// Ignore other cases
				file.ignore(numeric_limits<streamsize>::max(), '\n');		// Ignore the rest of the line
			}
		}
		file.close();

		out_vertices = raw_vertices;

		return true;
	}

	//----------------------------
	//----    CAMERA CLASS    ----
	//----------------------------

	const float SimpleCamera::min_pitch = -1.5f;
	const float SimpleCamera::max_pitch = 1.5f;
	const float SimpleCamera::min_distance = 1.0f;
	const float SimpleCamera::angle_sensitivity = 0.008f;
	const float SimpleCamera::zoom_sensitivity = 0.003f;

	SimpleCamera::SimpleCamera()
		: yaw(0.0f), pitch(0.0f), distance(5.0f), last_x(0), last_y(0), is_rotating(false), is_zooming(false), center(0.0f, 0.0f, 0.0f)
	{
		update_matrices();
	}

	void SimpleCamera::update_matrices()
	{
		// Compute the view_inv matrix. It is the world matrix of the camera object,
		// it describes the position of the camera in the scene.

		view_inv = glm::mat4(1.0f);
		view_inv = glm::translate(view_inv, center);
		view_inv = glm::rotate(view_inv, -yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		view_inv = glm::rotate(view_inv, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		view_inv = glm::translate(view_inv, glm::vec3(0.0f, 0.0f, distance));

		view = glm::inverse(view_inv);

		eye_position = glm::vec3(view_inv[3]);
	}

	void SimpleCamera::SetCamera(float yaw, float pitch, float distance)
	{
		this->yaw = yaw;
		this->pitch = pitch;
		this->distance = distance;

		// Clamp the data
		if (this->pitch > max_pitch)		this->pitch = max_pitch;
		if (this->pitch < min_pitch)		this->pitch = min_pitch;
		if (this->distance < min_distance)	this->distance = min_distance;

		update_matrices();
	}

	void SimpleCamera::SetCenter(const glm::vec3 &center)
	{
		this->center = center;
		update_matrices();
	}

	void SimpleCamera::OnMouseFunc(int button, int state, int x, int y)
	{
		// Left mouse button affects the angles
		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				last_x = x;
				last_y = y;
				is_rotating = true;
			}
			else is_rotating = false;
		}
		// Right mouse button affects the zoom
		if (button == GLUT_RIGHT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				last_x = x;
				last_y = y;
				is_zooming = true;
			}
			else is_zooming = false;
		}
	}

	void SimpleCamera::OnMotionFunc(int x, int y)
	{
		float dx = float(x - last_x);
		float dy = float(y - last_y);
		last_x = x;
		last_y = y;

		if (is_rotating)
		{
			yaw += dx * angle_sensitivity;
			pitch += -dy * angle_sensitivity;

			// Clamp the results
			if (pitch > max_pitch)	pitch = max_pitch;
			if (pitch < min_pitch)	pitch = min_pitch;
		}
		if (is_zooming)
		{
			distance *= (1.0f + dy * zoom_sensitivity);
			
			// Clamp the results
			if (distance < min_distance)		distance = min_distance;
		}
		if (is_rotating || is_zooming)
			update_matrices();
	}

	glm::mat4 SimpleCamera::GetViewMatrix() const
	{
		return view;
	}

	glm::mat4 SimpleCamera::GetViewInvMatrix() const
	{
		return view_inv;
	}

	glm::vec3 SimpleCamera::GetEyePosition() const
	{
		return eye_position;
	}

	//------------------------
	//----    TEXTURES    ----
	//------------------------

	void InitDevIL()
	{
		ilInit();
	}

	bool LoadAndSetTexture(const wchar_t *filename, GLenum target)
	{
		// Create IL image
		ILuint IL_tex;
		ilGenImages(1, &IL_tex);

		ilBindImage(IL_tex);

		// Solve upside down textures
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 

		// Load IL image
		ILboolean success = ilLoadImage(filename);
		if (!success)
		{
			ilBindImage(0);
			ilDeleteImages(1, &IL_tex); 
			printf("Couldn't load texture: %S\n", filename);
			return false;
		}

		// Get IL image parameters
		int img_width = ilGetInteger(IL_IMAGE_WIDTH);
		int img_height = ilGetInteger(IL_IMAGE_HEIGHT);
		int img_format = ilGetInteger(IL_IMAGE_FORMAT);
		int img_type = ilGetInteger(IL_IMAGE_TYPE);

		// Choose internal format, format, and type for glTexImage2D
		GLint internal_format = 0;
		GLenum format = 0;
		GLenum type = img_type;			// IL constants matches GL constants
		switch (img_format)
		{
		case IL_RGB:	internal_format = GL_RGB;	format = GL_RGB;	break;
		case IL_RGBA:	internal_format = GL_RGBA;	format = GL_RGBA;	break;
		case IL_BGR:	internal_format = GL_RGB;	format = GL_BGR;	break;
		case IL_BGRA:	internal_format = GL_RGBA;	format = GL_BGRA;	break;
		case IL_COLOR_INDEX:
		case IL_ALPHA:
		case IL_LUMINANCE:
		case IL_LUMINANCE_ALPHA:
			// Unsupported format
			ilBindImage(0);
			ilDeleteImages(1, &IL_tex); 
			printf("Texture %S has unsupported format\n", filename);
			return false;
		}

		// Set the data to OpenGL (assumes texture object is already bound)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(target, 0, internal_format, img_width, img_height, 0, format, type, ilGetData());

		// Unset and delete IL texture
		ilBindImage(0);
		ilDeleteImages(1, &IL_tex);

		return true;
	}

	GLuint CreateAndLoadTexture2D(const wchar_t *filename)
	{
		// Create OpenGL texture object
		GLuint tex_obj;
		glGenTextures(1, &tex_obj);
		glBindTexture(GL_TEXTURE_2D, tex_obj);

		// Load the data into OpenGL texture object
		if (!LoadAndSetTexture(filename, GL_TEXTURE_2D))
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &tex_obj);
			return 0;
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		return tex_obj;
	}

	GLuint CreateAndLoadTextureCube(
		const wchar_t *filename_px, const wchar_t *filename_nx,
		const wchar_t *filename_py, const wchar_t *filename_ny,
		const wchar_t *filename_pz, const wchar_t *filename_nz)
	{
		// Create OpenGL texture object
		GLuint tex_obj;
		glGenTextures(1, &tex_obj);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_obj);

		// Load the data into OpenGL texture object
		if (
			!LoadAndSetTexture(filename_px, GL_TEXTURE_CUBE_MAP_POSITIVE_X) ||
			!LoadAndSetTexture(filename_nx, GL_TEXTURE_CUBE_MAP_NEGATIVE_X) ||
			!LoadAndSetTexture(filename_py, GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
			!LoadAndSetTexture(filename_ny, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) ||
			!LoadAndSetTexture(filename_pz, GL_TEXTURE_CUBE_MAP_POSITIVE_Z) ||
			!LoadAndSetTexture(filename_nz, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z))
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDeleteTextures(1, &tex_obj);
			return 0;
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return tex_obj;
	}

	void SetTexture2DParameters(GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	}

	void SetTextureCubeParameters(GLint wrap_s, GLint wrap_t, GLint wrap_r, GLint min_filter, GLint mag_filter)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter);
	}

	//----------------------------
	//----    FRAMEBUFFERS    ----
	//----------------------------

	bool CheckFramebufferStatus(const char *desc)
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Failed to create a complete framebuffer (" << desc << ")" << endl;
			return false;
		}
		else return true;
	}

}