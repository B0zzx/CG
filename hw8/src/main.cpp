#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "draw.h"

using namespace std;



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


float xCur, yCur;	

Draw draw;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main() {\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"	ourColor = aColor;\n"
"}\0";


const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main() {\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n\0";



int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW8", NULL, NULL);

	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);		
	glfwSetMouseButtonCallback(window, mouse_button_callback);	

	gl3wInit();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram(); 
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(shaderProgram);

	ImVec4 line_color = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
	ImVec4 point_color = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	ImVec4 tangent_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < pointSet.size(); i++) {
			draw.drawPoint(shaderProgram, pointSet[i].x, pointSet[i].y, 5.0f, point_color);
			if (i > 0)
				draw.drawLine(shaderProgram, pointSet[i - 1].x, pointSet[i - 1].y, pointSet[i].x, pointSet[i].y, point_color);
		}
		if (pointSet.size() > 1)
			draw.drawBezierCurve(shaderProgram, line_color);
		float t = (float)((int)(glfwGetTime() * 10) % 16) / 16;
		if (pointSet.size() > 1)
			draw.drawTangent(shaderProgram, pointSet, t, tangent_color);
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();
	return 0;
}


void processInput(GLFWwindow * window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


void mouse_move_callback(GLFWwindow * window, double xpos, double ypos) {
	xCur = (xpos - SCR_WIDTH / 2) / SCR_WIDTH * 2;
	yCur = 0 - (ypos - SCR_HEIGHT / 2) / SCR_HEIGHT * 2;
};

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		Point point;
		point.setxy(xCur, yCur);
		pointSet.push_back(point);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (!pointSet.empty())
			pointSet.pop_back();
	}
}


void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
}
