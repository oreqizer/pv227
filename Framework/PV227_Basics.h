#pragma once
#ifndef INCLUDED_PV227_BASICS_H
#define INCLUDED_PV227_BASICS_H

// Include the most important libraries for our lessons

//	- Include standard libraries
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>

//	- Include GLEW, use static library
#define GLEW_STATIC
#include <GL/glew.h>

//	- Include FreeGLUT
#include <GL/freeglut.h>

//	- Include the most important GLM functions
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/color_space.hpp>

//	- Include AntTweakBar library
#include <AntTweakBar.h>

namespace PV227
{

	//------------------------------
	//----    SOME CONSTANTS    ----
	//------------------------------

	/// Default indices of input vertex shader variables, as we will use in PV227 lectures.
	/// Make sure this corresponds to layout (location=N) in shaders (or use glBindAttribLocation).
	const int DEFAULT_POSITION_LOC = 0;
	const int DEFAULT_NORMAL_LOC = 1;
	const int DEFAULT_TEX_COORD_LOC = 2;
	const int DEFAULT_TANGENT_LOC = 3;
	const int DEFAULT_BITANGENT_LOC = 4;

	/// Default binding points of basic uniform blocks, as we will use in PV227 lectures.
	/// Make sure this corresponds to layout (binding=N) in shaders (or use glUniformBlockBinding).
	const int DEFAULT_CAMERA_BINDING = 0;			// UBO with the data of the camera, like view/projection matrices
	const int DEFAULT_LIGHTS_BINDING = 1;			// UBO with the data of the lights, like their position or count
	const int DEFAULT_OBJECT_BINDING = 2;			// UBO with the data of the object, like its model matrix
	const int DEFAULT_MATERIAL_BINDING = 3;			// UBO with the data of the material, like its color

	//---------------------------
	//----    APPLICATION    ----
	//---------------------------

	/// Prints Vendor, Renderer and Version to standard output
	void PrintOpenGLInfo();

	/// Returns true if given OpenGL extension is present
	bool IsOpenGLExtensionPresent(const char *extension);

	/// Uses the proper Win/Linux/MacOS functions to set the debug callback method
	void SetDebugCallback(GLDEBUGPROCARB callback);

	/// Turns V-Sync on/off
	void SetVSync(bool use_v_sync);

	/// Returns the current FPS
	///
	/// This function measures the time between each of its calls, and estimates FPS from it.
	/// Call it exactly once per frame.
	float GetFPS();

	//------------------------------------
	//----    SHADERS AND PROGRAMS    ----
	//------------------------------------

	/// Loads a file and returns its content as std::string
	std::string LoadFileToString(const char *file_name);

	/// Creates a shader of given type, loads and sets its source code, compiles it, and prints errors
	/// to stdout if some occur.
	///
	/// Returns shader object on success or 0 if failed.
	GLuint LoadAndCompileShader(GLenum shader_type, const char *file_name);

	/// This is a VERY SIMPLE class that maintains a shader program and its shaders.
	/// It is not a perfect, brilliant, smart, or whatever implementation of a shader program.
	///
	/// To use it, follow these steps:
	///		ShaderProgram my_program;				// New object
	///		my_program.Init();						// Initialize
	///		my_program.AddShader(...);				// Add all shaders, call multiple times, once for each shader
	///		my_program.BindAttribLocation(...);		// Bind locations of all vertex shader inputs, call multiple times
	///		my_program.BindFragDataLocation(...);	// Bind locations of all fragment shader outputs, call multiple times
	///		my_program.Link();						// Link program
	///		if (my_program.IsValid())				// Test if the program is ready to use
	///
	/// There are also several methods that makes the work easier, especially (see their docs for more info):
	///		Use, GetAttribLocation, GetUniformLocation, GetUniformBlockIndex, SetUniformBlockBindingIndex, Uniform*
	class ShaderProgram
	{
	private:
		struct Shader
		{
			GLuint shader;				// OpenGL object of the shader
			GLenum type;				// Type of the shader, e.g. GL_VERTEX_SHADER
			const char *file_name;		// Path to the source of the shader
		};

		/// List of all shaders that the program uses
		std::vector<Shader> shaders;

		/// OpenGL object of the shader program
		GLuint program;

		/// True if this shader is ready to use
		bool valid;

	public:
		/// Initializes this object. It does not initialize OpenGL objects, because OpenGL may not be initialized here.
		ShaderProgram();

		// No destructor, especially no destructor that would automatically call Destroy, since OpenGL may be already unloaded.

		/// Initialize OpenGL objects. Call this when OpenGL context is created, i.e. after creating a window.
		void Init();
		/// Destroys all OpenGL objects and makes this program invalid.
		void Destroy();

		/// Loads, compiles, and adds given shader. When this fails, the program is destroyed,
		/// making all other AddShader/Bind*/Link/... not function.
		///
		/// Returns true if everything is OK, false if something failed.
		///
		/// Example of use: my_program.AddShader(GL_VERTEX_SHADER, "my_vertex_shader.glsl");
		bool AddShader(GLenum shader_type, const char *file_name);

		/// Shortcuts to AddShader, only calls AddShader with the proper shader_type
		///
		/// Example of use: my_program.AddVertexShader("my_vertex_shader.glsl");
		bool AddVertexShader(const char *file_name);
		bool AddTessControlShader(const char *file_name);
		bool AddTessEvaluationShader(const char *file_name);
		bool AddGeometryShader(const char *file_name);
		bool AddFragmentShader(const char *file_name);
		bool AddComputeShader(const char *file_name);

		/// Similar to glBindAttribLocation, uses this program as the first parameter.
		void BindAttribLocation(GLint idx, const char *name) const;
		/// Similar to glBindFragDataLocation, uses this program as the first parameter.
		void BindFragDataLocation(GLint idx, const char *name) const;

		/// Links the program and checks for errors. When this fails, the errors are printed to
		/// stdout and the program is destroyed.
		///
		/// Returns true if everything is OK, false if something failed.
		bool Link();

		/// Returns true if the shader program is linked and ready to use, false if it is not.
		bool IsValid() const;

		/// Returns the OpenGL object of the shader program.
		///
		/// Note that the object may be non-zero even when IsValid returns false. This happens
		/// mainly between calls to Init and Link. For this reason, use IsValid to check whether
		/// the program is ready to use.
		GLuint GetProgram() const;

		/// Similar to glGetAttribLocation, uses this program as the first parameter.
		GLint GetAttribLocation(const char *name) const;
		/// Similar to glGetUniformLocation, uses this program as the first parameter.
		GLint GetUniformLocation(const char *name) const;
		/// Similar to glGetUniformBlockIndex, uses this program as the first parameter.
		GLuint GetUniformBlockIndex(const char *name) const;

		/// Calls glUniformBlockBinding and glGetUniformBlockIndex to set the index of an uniform block
		///
		/// Example: my_program.SetUniformBlockBindingIndex("LightsData", 0);
		///				is the same as
		///			 layout (binding = 0) uniform LightsData
		void SetUniformBlockBindingIndex(const char *name, GLuint idx) const;

		/// Calls glUseProgram with this program as the parameter
		void Use() const;

		/// Similar to glUniform*(my_program.GetUniformLocation(name), ...);
		///
		/// These functions are very easy to use. However, beware of several caveats:
		///		- these functions do not call glUseProgram, make sure the program is active
		///		- these functions look for the location of the uniform variable each time
		///			they are called, which is not good, but sufficient for PV227 lectures
		///
		/// These functions are best for setting texture units to samplers, since they are
		/// usually set only once.
		///
		/// Example: my_program.Uniform1i("wood_tex", 0);
		void Uniform1f			(const char *name, GLfloat v0) const;
		void Uniform1fv			(const char *name, GLsizei count, const GLfloat* value) const;
		void Uniform1i			(const char *name, GLint v0) const;
		void Uniform1iv			(const char *name, GLsizei count, const GLint* value) const;
		void Uniform2f			(const char *name, GLfloat v0, GLfloat v1) const;
		void Uniform2fv			(const char *name, GLsizei count, const GLfloat* value) const;
		void Uniform2i			(const char *name, GLint v0, GLint v1) const;
		void Uniform2iv			(const char *name, GLsizei count, const GLint* value) const;
		void Uniform3f			(const char *name, GLfloat v0, GLfloat v1, GLfloat v2) const;
		void Uniform3fv			(const char *name, GLsizei count, const GLfloat* value) const;
		void Uniform3i			(const char *name, GLint v0, GLint v1, GLint v2) const;
		void Uniform3iv			(const char *name, GLsizei count, const GLint* value) const;
		void Uniform4f			(const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const;
		void Uniform4fv			(const char *name, GLsizei count, const GLfloat* value) const;
		void Uniform4i			(const char *name, GLint v0, GLint v1, GLint v2, GLint v3) const;
		void Uniform4iv			(const char *name, GLsizei count, const GLint* value) const;
		void UniformMatrix2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix2x3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix2x4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix3x2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix3x4fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix4x2fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
		void UniformMatrix4x3fv	(const char *name, GLsizei count, GLboolean transpose, const GLfloat* value) const;
	};

	/// ShaderProgramMap manages a group of shaders that are used to render the same objects in differennt
	/// situations (in different passes, in forward and deferred shading etc.). The usage is as follows:
	///
	///		ShaderProgramMap shader_map;
	///
	///		// Add shaders for each situation
	///		shader_map.AddProgram(0, &forward_shading_program);
	///		shader_map.AddProgram(1, &deferred_shading_program);
	///
	///		// Use shaders: for forward shading:
	///		if (shader_map.UseProgram(0))
	///		{
	///			...		// Render the object
	///		}
	///		// Use shaders: for deferred shading:
	///		if (shader_map.UseProgram(1))
	///		{
	///			...		// Render the object
	///		}
	class ShaderProgramMap
	{
	private:
		std::map<int, ShaderProgram *> shader_map;

	public:
		/// Adds the shader program into the map
		void AddProgram(int key, ShaderProgram *program);
		/// Removes the shader program from the map
		void RemoveProgram(int key);
		/// Removes all shader programs from the map
		void RemoveAll();

		/// Returns a shader program at given key or nullptr if no such program exists.
		ShaderProgram *GetProgram(int key);

		/// Tries to use the shader program. If the shader program exists and is valid, it is used and
		/// the function returns true. If the shader program is not present or it is not valid, it is not
		/// used (obviously) and false is returned.
		bool UseProgram(int key);
	};

	//------------------------------
	//----    GEOMETRY CLASS    ----
	//------------------------------

	/// This is a VERY SIMPLE class to contain all buffers and vertex array objects for geometries of
	/// our lectures. It is not a perfect, brilliant, smart, or whatever implementation of a geometry.
	///
	/// Although this is a class, it has no private attributes. We know it is not good and safe, but
	/// it is fast and perfect for prototyping.
	///
	/// To draw the geometry, bind this geometry's VAO (using glBindVertexArray(geom.VAO) or geom.BindVAO()),
	/// and call drawing commands using DrawGeometry() or DrawGeometryInstanced().
	///
	/// Example:
	///		my_cube = CreateCube(...);
	///		...
	///		my_cube.BindVAO();
	///		my_cube.Draw();
	class Geometry
	{
	public:
		//--  Attributes  --

		/// Buffers with the data of the geometry (positions, normals, texture coordinates, etc.).
		std::vector<GLuint> VertexBuffers;

		/// Buffer with the indices of the geometry
		GLuint IndexBuffer;

		/// Vertex Array Object with the geometry
		GLuint VAO;

		/// Type of the primitives to be drawn, e.g. GL_TRIANGLES
		GLenum Mode;
		/// Number of patch vertices, used when Mode is GL_PATCHES, ignored otherwise
		GLsizei PatchVertices;
		/// Number of vertices to be drawn using glDrawArrays
		GLsizei DrawArraysCount;
		/// Number of vertices to be drawn using glDrawElements
		GLsizei DrawElementsCount;

		//--  Methods  --

		/// Initializes this object. It does not initialize OpenGL objects, because OpenGL may not be initialized here.
		Geometry();

		/// Copy constructor, assign operator. They do only a shallow copy (they copy the IDs), they
		/// do not create new objects.
		Geometry(const Geometry &rhs);
		Geometry &operator =(const Geometry &rhs);

		// No destructor, especially no destructor that would automatically call Destroy, since OpenGL may be already unloaded.

		// No Init(), objects are initialized by functions that creates the geometry, see for example CreateCube function.

		/// Deletes all OpenGL objects of the geometry, i.e. deletes all buffers in VertexBuffers array,
		/// IndexBuffer, VAO, and resets DrawArraysCount and DrawElementsCount to zero.
		void Destroy();

		/// Binds this geometry's VAO
		void BindVAO() const;

		/// Chooses glDrawArrays or glDrawElements to draw the geometry.
		void Draw() const;
		/// Chooses glDrawArraysInstanced or glDrawElementsInstanced to draw multiple instances of the geometry.
		void DrawInstanced(int primcount) const;
	};

	/// Creates simple objects:
	///		- Cube: The center of the cube is in (0,0,0) and the length of its side is 2
	///			(positions of its vertices are from -1 to 1).
	///		- Sphere: The center of the sphere is in (0,0,0) and its radius is 1
	///			(positions of its vertices are from -1 to 1).
	///		- Torus: The center of the torus is in (0,0,0), its height is 1 (y coordinates are from -0.5 to 0.5),
	///			and its width is 3 (x/z coordinates are from -1.5 to 1.5),
	///		- Cylinder: The center of the cylinder is in (0,0,0), its height is 2 (y coordinates are from -1 to 1),
	///			and its radius is 0.5 (x/z coordinates are from -0.5 to 0.5),
	///		- Capsule: The center of the capsule is in (0,0,0), its height is 3 (y coordinates are from -1.5 to 1.5),
	///			and its radius is 0.5 (x/z coordinates are from -0.5 to 0.5),
	///		- Teapot: The center of the bottom of its body is roughly in (0,0,0) and the radius of the body is roughly 1.
	///			Its handle is in -X direction, its spout is in +X direction, and its lid is in +Y direction.
	///		- TeapotPatch: The same as teapot, except that it is defined by 32 Bezier patches, each with 16 vertices.
	///			The first four vertices of each patch go from (0,0) - (1,0), the next four vertices from (0,1/3) - (1,1/3) etc.
	///
	/// 'position_loc', 'normal_loc', 'tex_coord_loc', 'tangent_loc' and 'bitangent_loc' are locations
	/// of vertex attributes, obtained by glGetAttribLocation. Use -1 if not necessary.
	Geometry CreateCube(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateSphere(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateTorus(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateCylinder(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateCapsule(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateTeapot(GLint position_loc = DEFAULT_POSITION_LOC, GLint normal_loc = DEFAULT_NORMAL_LOC,
		GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	Geometry CreateTeapotPatch(GLint position_loc = DEFAULT_POSITION_LOC, GLint tangent_loc = DEFAULT_TANGENT_LOC, GLint bitangent_loc = DEFAULT_BITANGENT_LOC);
	/// Creates a geometry for a fullscreen quad for postprocessing: an empty VAO and a draw call which renders a single triangle
	Geometry CreateFullscreenQuad();

	/// Loads an OBJ file and creates a corresponding Geometry object. It is a very simple loader,
	/// it handles only triangles, it does not handle MTL files etc.
	///
	/// TODO: Do we need other file formats?
	///			- Yes, we want skinning.
	/// TODO: What OBJ files will we use?
	/// TODO: Do we need OBJ loader?
	///
	/// 'position_loc', 'normal_loc', and 'tex_coord_loc' are locations of vertex attributes,
	/// obtained by glGetAttribLocation. Use -1 if not necessary.
	Geometry LoadOBJ(const char *file_name, GLint position_loc = DEFAULT_POSITION_LOC,
		GLint normal_loc = DEFAULT_NORMAL_LOC, GLint tex_coord_loc = DEFAULT_TEX_COORD_LOC);

	/// Loads only vertices ('v') from given OBJ file, and stores them into given vector.
	///
	/// Returns true on success, false if the file cannot be opened.
	bool LoadVerticesFromOBJFile(const char *file_name, std::vector<glm::vec3> &out_vertices);
	
	//----------------------------
	//----    CAMERA CLASS    ----
	//----------------------------

	/// This is a VERY SIMPLE class that allows to very simply move with the camera.
	/// It is not a perfect, brilliant, smart, or whatever implementation of a camera,
	/// but it is sufficient for our lectures.
	///
	/// Use left mouse button to change the point of view.
	/// Use right mouse button to zoom in and zoom out.
	class SimpleCamera
	{
	private:
		/// Constants that defines the behaviour of the camera
		///		- Minimum/maximum pitch in radians
		static const float min_pitch, max_pitch;
		///		- Minimum distance from the point of interest
		static const float min_distance;
		///		- Sensitivity of the mouse when changing yaw or pitch angles
		static const float angle_sensitivity;
		///		- Sensitivity of the mouse when changing zoom
		static const float zoom_sensitivity;

		/// yaw is an angle which determines into which direction in xz plane I look.
		///		- 0 degrees .. I look into -z direction
		///		- 90 degrees .. I look into +x direction
		///		- 180 degrees .. I look into +z direction
		///		- 270 degrees .. I look into -x direction
		float yaw;
		
		/// pitch is an angle in which determines from which "height" I look.
		///		- positive pitch .. I look up, from "below" the xz plane
		///		- negative pitch .. I look down, from "above" the xz plane
		float pitch;

		/// Distance from (0,0,0), the point at which I look
		float distance;

		/// Center point at which the camera looks
		glm::vec3 center;

		/// Final view matrix, inverse of the view matrix, and the position of the eye in world space coordinates
		glm::mat4 view;
		glm::mat4 view_inv;
		glm::vec3 eye_position;

		/// Last X and Y coordinates of the mouse cursor
		int last_x, last_y;

		/// True or false if moused buttons are pressed and the user rotates/zooms the camera
		bool is_rotating, is_zooming;

		/// Recomputes 'view', 'view_inv', and 'eye_position' from 'angle_direction', 'angle_elevation', and 'distance'
		void update_matrices();

	public:
		SimpleCamera();

		/// Sets camera position
		void SetCamera(float yaw, float pitch, float distance);

		/// Sets the center point around which the camera moves
		void SetCenter(const glm::vec3 &center);

		/// Reacts to glutMouseFunc
		void OnMouseFunc(int button, int state, int x, int y);
		/// Reacts to glutMotionFunc
		void OnMotionFunc(int x, int y);

		/// Returns the view matrix, inverse of the view matrix, and the position of the eye in world space coordinates
		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetViewInvMatrix() const;
		glm::vec3 GetEyePosition() const;
	};

	//------------------------
	//----    TEXTURES    ----
	//------------------------

	/// Initialize DevIL library (actually, it only hides ilInit() from outside)
	void InitDevIL();

	/// Loads a texture from file and calls glTexImage2D to set its data using a given target.
	/// Returns true on success or false on failure.
	bool LoadAndSetTexture(const wchar_t *filename, GLenum target);

	/// Loads a 2D image, creates a new texture object, and sets the image to it.
	/// Returns the texture object on success, 0 on failure.
	GLuint CreateAndLoadTexture2D(const wchar_t *filename);

	/// Loads six 2D images representing sides of a cube texture, creates a new texture object, and sets the images to it.
	/// Returns the texture object on success, 0 on failure.
	GLuint CreateAndLoadTextureCube(
		const wchar_t *filename_px, const wchar_t *filename_nx,
		const wchar_t *filename_py, const wchar_t *filename_ny,
		const wchar_t *filename_pz, const wchar_t *filename_nz);

	/// Sets GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_MIN_FILTER, and GL_TEXTURE_MAG_FILTER parameters
	/// of a currently bound 2D texture. 
	void SetTexture2DParameters(GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter);

	/// Sets GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_MIN_FILTER, and GL_TEXTURE_MAG_FILTER parameters
	/// of a currently bound 2D texture. 
	void SetTextureCubeParameters(GLint wrap_s, GLint wrap_t, GLint wrap_r, GLint min_filter, GLint mag_filter);

	//----------------------------
	//----    FRAMEBUFFERS    ----
	//----------------------------

	/// When setting a framebuffer, we need to "connect" fragment output data with color attachments. We do this by calling
	/// glDrawBuffers with {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, ...} as the second parametr. The following constant
	/// just saves repeating the definition of the array again and again.
	const GLenum DrawBuffersConstants[] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};

	/// Checks the status of the currently bound GL_FRAMEBUFFER. If it is OK, this function returns true. If it fails,
	/// this function prints an error message using 'desc' as a description of the framebuffer, and returns false.
	bool CheckFramebufferStatus(const char *desc);

}

#endif	// INCLUDED_PV227_BASICS_H