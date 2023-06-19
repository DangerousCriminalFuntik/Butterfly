#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <string_view>
#include <tuple>
#include <random>
#include <chrono>

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
};

struct Object // Create A Structure Called Object
{
	int tex; // Integer Used To Select Our Texture
	float x; // X Position
	float y; // Y Position
	float z; // Z Position
	float yi; // Y Increase Speed (Fall Speed)
	float spinz; // Z Axis Spin
	float spinzi; // Z Axis Spin Speed
	float flap; // Flapping Triangles :)
	float fi; // Flap Direction (Increase Value)
};

std::vector<Object> obj(50); // Create 50 Objects Using The Object Structure

void SetObject(int loop) // Sets The Initial Value Of Each Object (Random)
{
	std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count()); // Seed random number generator
	std::uniform_int_distribution<int> textureDist(0, 2);
	std::uniform_real_distribution<float> xDist(-17.0f, 17.0f);
	std::uniform_real_distribution<float> zDist(-40.0f, -10.0f);
	std::uniform_real_distribution<float> spinziDist(-1.0f, 1.0f);
	std::uniform_real_distribution<float> fiDist(0.05f, 0.15f);
	std::uniform_real_distribution<float> yiDist(0.001f, 0.101f);

	obj[loop].tex = textureDist(rng); // Texture Can Be One Of 3 Textures
	obj[loop].x = xDist(rng); // Random x Value From -17.0f To 17.0f
	obj[loop].y = 18.0f; // Set y Position To 18 (Off Top Of Screen)
	obj[loop].z = zDist(rng); // z Is A Random Value From -10.0f To -40.0f
	obj[loop].spinzi = spinziDist(rng); // spinzi Is A Random Value From -1.0f To 1.0f
	obj[loop].flap = 0.0f; // flap Starts Off At 0.0f;
	obj[loop].fi = fiDist(rng); // fi Is A Random Value From 0.05f To 0.15f
	obj[loop].yi = yiDist(rng); // yi Is A Random Value From 0.001f To 0.101f
}

std::vector<Vertex> vertices = {
	// First triangle
	Vertex{glm::vec3(1.0f,1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(-1.0f,1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(-1.0f,-1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
	// Second triangle
	Vertex{glm::vec3(1.0f,1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(-1.0f,-1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(1.0f,-1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}
};

std::vector<GLuint> indices = {
	0, 1, 2, // Triangle 1
	3, 4, 5  // Triangle 2
};

// Function Prototypes
void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void checkShader(GLuint shader);
void checkProgram(GLuint program);
std::string readFile(std::string_view filename);
GLuint createShader(std::string_view filename, GLenum shaderType);
std::tuple<GLuint, GLuint> compileProgram(std::array<std::string_view, 2> const& sources);
GLuint createTexture2D(GLenum internalFormat, GLenum format, GLsizei width, GLsizei height, const void* data = nullptr,
	GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter = GL_LINEAR, GLenum wrapMode = GL_REPEAT);
using stb_comp_t = decltype(STBI_default);
GLuint loadTexture(std::string_view filename, stb_comp_t comp = STBI_rgb_alpha);


int main()
{
	if (!glfwInit())
		return 1;

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Butterfly", nullptr, nullptr);
	if (!window)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return 1;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	const auto [program, pipeline] = compileProgram({ "shader.vert", "shader.frag" });
	GLint mvp_loc = glGetProgramResourceLocation(program, GL_UNIFORM, "mvpMatrix");
		
	GLuint vbo[2];
	glCreateBuffers(2, vbo);
	glNamedBufferStorage(vbo[0], vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(vbo[1], indices.size() * sizeof(GLuint), indices.data(), 0);
	
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(vao, 0);

	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayVertexBuffer(vao, 0, vbo[0], 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vao, vbo[1]);

	GLuint texture[3];
	texture[0] = loadTexture("Butterfly1.png");
	texture[1] = loadTexture("Butterfly2.png");
	texture[2] = loadTexture("Butterfly3.png");

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (size_t i = 0; i < obj.size(); ++i) // Loop To Initialize 50 Objects
	{
		SetObject(i);	// Call SetObject To Assign New Random Values
	}
						
	while (!glfwWindowShouldClose(window))
	{
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 0.5f)[0]);
		glClearBufferfv(GL_DEPTH, 0, &glm::vec4(1.0f)[0]);

		auto aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
		glm::mat4 modelViewMatrix = glm::mat4(1.0f);

		for (size_t i = 0; i < obj.size(); ++i)
		{
			modelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(obj[i].x, obj[i].y, obj[i].z));
			modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate On The X-Axis
			modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(obj[i].spinz), glm::vec3(0.0f, 0.0f, 1.0f)); // Spin On The Z-Axis

			// Update the vertices based on obj[i].flap
			vertices[1].position.z = obj[i].flap;
			vertices[5].position.z = obj[i].flap;
			
			// Upload the updated vertex data to the VBO
			glNamedBufferSubData(vbo[0], 0, vertices.size() * sizeof(Vertex), vertices.data());
			
			glBindProgramPipeline(pipeline);
			glProgramUniformMatrix4fv(program, mvp_loc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * modelViewMatrix));

			// Bind the texture
			glBindTextureUnit(0, texture[obj[i].tex]);

			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
			
			obj[i].y -= obj[i].yi; // Move Object Down The Screen
			obj[i].spinz += obj[i].spinzi; // Increase Z Rotation By spinzi
			obj[i].flap += obj[i].fi; // Increase flap Value By fi

			if (obj[i].y < -18.0f) // Is Object Off The Screen?
			{
				SetObject(i); // If So, Reassign New Values
			}

			if ((obj[i].flap > 1.0f) || (obj[i].flap < -1.0f)) // Time To Change Flap Direction?
			{
				obj[i].fi = -obj[i].fi; // Change Direction By Making fi = -fi
			}
		}

		// Create A Short Delay (15 Milliseconds)
		std::this_thread::sleep_for(std::chrono::milliseconds(15));

		// Flush The GL Rendering Pipeline
		glFlush();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteProgram(program);
	glDeleteProgramPipelines(1, &pipeline);
	glDeleteBuffers(2, vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture[0]);
	glDeleteTextures(1, &texture[1]);
	glDeleteTextures(1, &texture[2]);
	
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void error_callback(int error, const char* description)
{
	std::cerr << "Error (" << error << "): " << description << "\n";
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLuint createShader(std::string_view filename, GLenum shaderType)
{
	auto source = readFile(filename);
	const GLchar* src = source.c_str();

	GLuint shader{ glCreateShader(shaderType) };
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	checkShader(shader);

	return shader;
}

std::tuple<GLuint, GLuint> compileProgram(std::array<std::string_view, 2> const& sources)
{
	auto vs = createShader(sources[0], GL_VERTEX_SHADER);
	auto fs = createShader(sources[1], GL_FRAGMENT_SHADER);
	static std::vector<GLuint> shaders{ vs, fs };

	GLuint program = glCreateProgram();
	glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);

	for (const auto& shader : shaders)
	{
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	checkProgram(program);

	for (const auto& shader : shaders)
	{
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}

	GLuint pipeline{};
	glCreateProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, program);

	return std::make_tuple(program, pipeline);
}

void checkShader(GLuint shader)
{
	GLint isCompiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	GLint maxLength{};
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
	
	if (maxLength > 0
#ifdef NDEBUG
		&& isCompiled == GL_FALSE
#endif // NDEBUG
		)
	{
		std::vector<char> buffer(maxLength);
		glGetShaderInfoLog(shader, maxLength, nullptr, buffer.data());
		glDeleteShader(shader);

		std::cerr << "Error compiled:\n" << buffer.data() << '\n';
	}
}

void checkProgram(GLuint program)
{
	GLint isLinked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

	GLint maxLength{};
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
	
	if (maxLength > 0
#ifdef NDEBUG
		&& isLinked == GL_FALSE
#endif // NDEBUG
		)
	{
		std::vector<char> buffer(maxLength);
		glGetProgramInfoLog(program, maxLength, nullptr, buffer.data());
		glDeleteProgram(program);

		std::cerr << "Error linking:\n" << buffer.data() << '\n';
	}
}

std::string readFile(std::string_view filename)
{
	std::string result;
	std::ifstream stream(filename.data());

	if (!stream.is_open())
	{
		std::cerr << "Failed to open file: " << filename << '\n';
		return result;
	}

	stream.seekg(0, std::ios::end);
	result.reserve((size_t)stream.tellg());
	stream.seekg(0, std::ios::beg);

	result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

	return result;
}

GLuint loadTexture(std::string_view filename, stb_comp_t comp /*= STBI_rgb_alpha*/)
{
	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;
	const auto data = stbi_load(filename.data(), &width, &height, &channels, comp);
	if (!data)
	{
		std::cerr << "Failed to load texture: " << filename << '\n';
	}

	const auto [in, ex] = [comp] {
		switch (comp)
		{
		case STBI_rgb_alpha:	return std::make_pair(GL_RGBA8, GL_RGBA);
		case STBI_rgb:			return std::make_pair(GL_RGB8, GL_RGB);
		case STBI_grey:			return std::make_pair(GL_R8, GL_RED);
		case STBI_grey_alpha:	return std::make_pair(GL_RG8, GL_RG);
		default:
			throw std::runtime_error("Invalid format!");
		}
	}();

	const auto name = createTexture2D(in, ex, width, height, data);
	stbi_image_free(data);

	return name;
}

GLuint createTexture2D(GLenum internalFormat, GLenum format, GLsizei width, GLsizei height, const void* data /*= nullptr*/,
	GLenum minFilter /*= GL_LINEAR_MIPMAP_LINEAR*/, GLenum magFilter /*= GL_LINEAR*/, GLenum wrapMode /*= GL_REPEAT*/)
{
	GLuint textureId{};
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
	glTextureStorage2D(textureId, 1, internalFormat, width, height);
	glTextureSubImage2D(textureId, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, magFilter);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, wrapMode);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, wrapMode);
	glGenerateTextureMipmap(textureId);

	return textureId;
}
