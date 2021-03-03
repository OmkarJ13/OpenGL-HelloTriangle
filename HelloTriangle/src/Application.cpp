#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include <iostream>

static float xOffset = 0.0f;
static const float maxOffset = 0.5f;
static bool direction = true;

struct ShaderProgramCode 
{
	std::string VertexCode;
	std::string FragmentCode;
};

static ShaderProgramCode ParseShader(const char* filePath)
{
	FILE* f = nullptr;
	fopen_s(&f, filePath, "r");
	if (!f)
	{
		std::cout << "Error opening file!" << std::endl;
	}
	
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer = (char*)alloca(len);
	*buffer = NULL;

	char* vertexSrc = (char*)alloca(len);
	*vertexSrc = NULL;

	char* fragmentSrc = (char*)alloca(len);
	*fragmentSrc = NULL;

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	ShaderType type = ShaderType::NONE;

	while (fgets(buffer, len, f))
	{
		if (strcmp(buffer, "#shader vertex\n") == 0)
		{
			type = ShaderType::VERTEX;
		}
		else if (strcmp(buffer, "#shader fragment\n") == 0)
		{
			type = ShaderType::FRAGMENT;
		}
		else
		{
			if (type == ShaderType::VERTEX)
			{
				strcat_s(vertexSrc, len, buffer);
			}
			else if (type == ShaderType::FRAGMENT)
			{
				strcat_s(fragmentSrc, len, buffer);
			}
		}
	}

	fclose(f);
	return { vertexSrc, fragmentSrc };
}

static uint32_t CompileShader(uint32_t type, const std::string& source)
{
	uint32_t shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		char* errorLog = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(shader, length, &length, errorLog);

		std::cout << "Failed to compile shaders!" << std::endl;
		std::cout << errorLog << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

static uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	uint32_t shaderProgram = glCreateProgram();
	
	uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);

	int result;

	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);

	if (!result)
	{
		int length;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);

		char* errorLog = (char*)alloca(length * sizeof(char));
		glGetProgramInfoLog(shaderProgram, length, &length, errorLog);

		std::cout << "Failed to link program!" << std::endl;
		std::cout << errorLog << std::endl;

		glDeleteProgram(shaderProgram);
		return 0;
	}

	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &result);

	if (!result)
	{
		int length;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);

		char* errorLog = (char*)alloca(length * sizeof(char));
		glGetProgramInfoLog(shaderProgram, length, &length, errorLog);

		std::cout << "Failed to validate program!" << std::endl;
		std::cout << errorLog << std::endl;

		glDeleteProgram(shaderProgram);
		return 0;
	}

	return shaderProgram;
}

static void ChangeOffset()
{
	direction ? xOffset += 0.005f : xOffset -= 0.005f;
	if (abs(xOffset) >= maxOffset)
	{
		direction = !direction;
	}
}

int main()
{
	if (!glfwInit())
	{
		std::cout << "GLFW initialisation failed!" << std::endl;
		glfwTerminate();
		return -1;
	}

	GLFWwindow* mainWindow = glfwCreateWindow(640, 480, "Hello Triangle", nullptr, nullptr);
	if (!mainWindow)
	{
		std::cout << "GLFW failed to create window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(mainWindow);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW initialisation failed!" << std::endl;
		glfwTerminate();
		return -1;
	}

	float positions[6] = {
		-0.5f, -0.5f,  // Left
		 0.0f,  0.5f,  // Center
		 0.5f, -0.5f   // Right
	};

	float colours[12] = {
		1.0f, 0.0f, 0.0f, 1.0f, // Left
		0.0f, 1.0f, 0.0f, 1.0f, // Center
		0.0f, 0.0f, 1.0f, 1.0f  // Right
	};

	uint32_t VAO;
	uint32_t VBO[2];

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(2, VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, colours, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	ShaderProgramCode code;
	code = ParseShader("../HelloTriangle/res/shaders/Base.shader");

	uint32_t shaderProgram = CreateShader(code.VertexCode, code.FragmentCode);
	uint32_t uniformModel = glGetUniformLocation(shaderProgram, "model");

	glfwSwapInterval(1);
	glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
	glEnable(GL_FRAMEBUFFER_SRGB);

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(xOffset, 0.0f, 0.0f));

		glBindVertexArray(VAO);
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUseProgram(0);
		glBindVertexArray(0);

		ChangeOffset();
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}

	std::cout << "Press enter to exit..." << std::endl;
	glfwTerminate();
	std::cin.get();
	return 0;
}