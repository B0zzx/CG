#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <string>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <time.h>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "camera.h"


using namespace std;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void window_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

const char* vertexShaderSourcePhong = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 fragPos;\n"
"out vec3 normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main() {\n"
"   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"	fragPos = vec3(model * vec4(aPos.x, aPos.y, aPos.z, 1.0));\n"
"	normal = aNormal;\n"
"}\0";


const char* fragmentShaderSourcePhong = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 normal;\n"
"in vec3 fragPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 lightColor;\n"
"uniform float ambientFactor;\n"
"uniform float diffuseFactor;\n"
"uniform float specularFactor;\n"
"uniform int shininessFactor;\n"
"void main() {\n"
"	vec3 norm = normalize(normal);\n"
"	vec3 lightDirection = normalize(lightPos - fragPos);\n"
"	float diff = max(dot(norm, lightDirection), 0.0);\n"
"	vec3 diffuse = diffuseFactor * diff * lightColor;\n"
"	vec3 viewDirection = normalize(viewPos - fragPos);\n"
"	vec3 reflectDirection = reflect(-lightDirection, norm);\n"
"	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininessFactor);\n"
"	vec3 specular = specularFactor * spec * lightColor;\n"
"	vec3 ambient = ambientFactor * lightColor;\n"
"	vec3 result = (ambient + diffuse + specular) * objectColor; \n"
"   FragColor = vec4(result, 1.0f);\n"
"}\n\0";

const char* vertexShaderSourceGouraud = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 lightingColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"uniform float ambientFactor;\n"
"uniform float diffuseFactor;\n"
"uniform float specularFactor;\n"
"uniform int shininessFactor;\n"
"void main() {\n"
"   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"	vec3 fragPos = vec3(model * vec4(aPos.x, aPos.y, aPos.z, 1.0));\n"
"	vec3 normal = aNormal;\n"
"	vec3 norm = normalize(normal);\n"
"	vec3 lightDirection = normalize(lightPos - fragPos);\n"
"	float diff = max(dot(norm, lightDirection), 0.0);\n"
"	vec3 diffuse = diffuseFactor * diff * lightColor;\n"
"	vec3 viewDirection = normalize(viewPos - fragPos);\n"
"	vec3 reflectDirection = reflect(-lightDirection, norm);\n"
"	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininessFactor);\n"
"	vec3 specular = specularFactor * spec * lightColor; \n"
"	vec3 ambient = ambientFactor * lightColor;\n"
"	lightingColor = ambient + diffuse + specular;\n"
"}\0";

const char* fragmentShaderSourceGouraud = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 lightingColor;\n"
"uniform vec3 objectColor;\n"
"void main() {\n"
"	FragColor = vec4(lightingColor * objectColor, 1.0f);\n"
"}\0";

const char* vertexShaderSourceLight = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main() {\n"
"   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSourceLight = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"	FragColor = vec4(1.0f);\n"
"}\0";

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_WIDTH, "HW4", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, window_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1);

	gl3wInit();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsClassic();

	bool ImGui = true;

	float vertices[216] = {
		-2.0f, -2.0f, -2.0f, 4.0f, 0.0f, 0.0f,
		2.0f, -2.0f, -2.0f, 4.0f, 0.0f, 0.0f,
		2.0f, 2.0f, -2.0f, 4.0f, 0.0f, 0.0f,
		2.0f, 2.0f, -2.0f, 4.0f, 0.0f, 0.0f,
		-2.0f, 2.0f, -2.0f, 4.0f, 0.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 4.0f, 0.0f, 0.0f,

		-2.0f, -2.0f, 2.0f, 0.0f, 0.0f, 4.0f,
		2.0f, -2.0f, 2.0f, 0.0f, 0.0f, 4.0f,
		2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 4.0f,
		2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 4.0f,
		-2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 4.0f,
		-2.0f, -2.0f, 2.0f, 0.0f, 0.0f, 4.0f,

		-2.0f, 2.0f, 2.0f, 0.0f, 4.0f, 0.0f,
		-2.0f, 2.0f, -2.0f, 0.0f, 4.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 4.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 4.0f, 0.0f,
		-2.0f, -2.0f, 2.0f, 0.0f, 4.0f, 0.0f,
		-2.0f, 2.0f, 2.0f, 0.0f, 4.0f, 0.0f,

		2.0f, 2.0f, 2.0f,4.0f, 0.0f, 4.0f,
		2.0f, 2.0f, -2.0f,4.0f, 0.0f, 4.0f,
		2.0f, -2.0f, -2.0f,4.0f, 0.0f, 4.0f,
		2.0f, -2.0f, -2.0f,4.0f, 0.0f, 4.0f,
		2.0f, -2.0f, 2.0f,4.0f, 0.0f, 4.0f,
		2.0f, 2.0f, 2.0f,4.0f, 0.0f, 4.0f,

		-2.0f, -2.0f, -2.0f, 4.0f, 4.0f, 0.0f,
		2.0f, -2.0f, -2.0f, 4.0f, 4.0f, 0.0f,
		2.0f, -2.0f, 2.0f,4.0f, 4.0f, 0.0f,
		2.0f, -2.0f, 2.0f,4.0f, 4.0f, 0.0f,
		-2.0f, -2.0f, 2.0f,4.0f, 4.0f, 0.0f,
		-2.0f, -2.0f, -2.0f,4.0f, 4.0f, 0.0f,

		-2.0f, 2.0f, -2.0f,0.0f, 4.0f, 4.0f,
		2.0f, 2.0f, -2.0f,0.0f, 4.0f, 4.0f,
		2.0f, 2.0f, 2.0f,0.0f, 4.0f, 4.0f,
		2.0f, 2.0f, 2.0f,0.0f, 4.0f, 4.0f,
		-2.0f, 2.0f, 2.0f,0.0f, 4.0f, 4.0f,
		-2.0f, 2.0f, -2.0f,0.0f, 4.0f, 4.0f,
	};


	for (int i = 0; i < 216; i++)
	{
		vertices[i] *= 0.25;
	}

	int vertexShaderPhong = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderPhong, 1, &vertexShaderSourcePhong, NULL);
	glCompileShader(vertexShaderPhong);

	int fragmentShaderPhong = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderPhong, 1, &fragmentShaderSourcePhong, NULL);
	glCompileShader(fragmentShaderPhong);

	int vertexShaderGouraud = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderGouraud, 1, &vertexShaderSourceGouraud, NULL);
	glCompileShader(vertexShaderGouraud);

	int fragmentShaderGouraud = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderGouraud, 1, &fragmentShaderSourceGouraud, NULL);
	glCompileShader(fragmentShaderGouraud);

	int vertexShaderLight = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderLight, 1, &vertexShaderSourceLight, NULL);
	glCompileShader(vertexShaderLight);

	int fragmentShaderLight = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderLight, 1, &fragmentShaderSourceLight, NULL);
	glCompileShader(fragmentShaderLight);

	int shaderProgramPhong = glCreateProgram();
	glAttachShader(shaderProgramPhong, vertexShaderPhong);
	glAttachShader(shaderProgramPhong, fragmentShaderPhong);
	glLinkProgram(shaderProgramPhong);

	int shaderProgramGouraud = glCreateProgram();
	glAttachShader(shaderProgramGouraud, vertexShaderGouraud);
	glAttachShader(shaderProgramGouraud, fragmentShaderGouraud);
	glLinkProgram(shaderProgramGouraud);

	int shaderProgramLight = glCreateProgram();
	glAttachShader(shaderProgramLight, vertexShaderLight);
	glAttachShader(shaderProgramLight, fragmentShaderLight);
	glLinkProgram(shaderProgramLight);

	unsigned int VBO;
	unsigned int VAO;
	unsigned int LightVAO;
	glGenBuffers(1, &VBO);


	glGenVertexArrays(1, &LightVAO);
	glBindVertexArray(LightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	int tag = 0;
	bool flag = true;
	bool camflag = false;
	bool lightflag = false;
	float radians = 45.0f;
	float distance = 100.0f;
	float para1 = -1.0f;
	float para2 = 1.0f;
	float para3 = -1.0f;
	float para4 = 1.0f;
	float locX = 3.0f;
	float locY = 3.0f;
	float locZ = 3.0f;

	float ambientFactor = 0.1f;
	float diffuseFactor = 1.0f;
	float specularFactor = 0.5f;
	int shininessFactor = 32;


	glEnable(GL_DEPTH_TEST);


	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		int view_width, view_height;
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);

		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::Begin("hw6", &ImGui, ImGuiWindowFlags_MenuBar);
		/*
		ImGui::RadioButton("perspective", &tag, 0);
		ImGui::RadioButton("ortho", &tag, 1);
		ImGui::RadioButton("rotation", &tag, 2);
		ImGui::RadioButton("move", &tag, 3);
		*/
		ImGui::Checkbox("Camera Move", &camflag);
		ImGui::Checkbox("Using Phone Shading", &flag);
		ImGui::Checkbox("Spinning Light", &lightflag);
		ImGui::SliderFloat("ambient", &ambientFactor, 0, 3);
		ImGui::SliderFloat("diffuse", &diffuseFactor, 0, 3);
		ImGui::SliderFloat("specular", &specularFactor, 0, 5);
		ImGui::SliderInt("shininess", &shininessFactor, 0, 256);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

		float camPosX = 2.0f;
		float camPosY = 2.0f;
		float camPosZ = 4.0f;
		glm::vec3 cameraPos = glm::vec3(camPosX, camPosY, camPosZ);
		glm::vec3 cameraFront = glm::vec3(-1.0f, -1.0f, -2.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		if (camflag) {
			view = camera.GetViewMatrix();
		}

		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		glm::vec3 lightPosition = glm::vec3(-0.7f, 1.0f, 0.7f);

		/*
		if (tag == 0)
		{
			ImGui::SliderFloat("radians", &radians, 10.0f, 80.0f);
			ImGui::SliderFloat("distance", &distance, 0.1f, 100.0f);
			projection = glm::perspective(glm::radians(radians), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, distance);
			float camPosX = 3.0f;
			float camPosY = 3.0f;
			float camPosZ = 3.0f;
			glm::vec3 cameraPos = glm::vec3(camPosX, camPosY, camPosZ);
			glm::vec3 cameraFront = glm::vec3(-1.5f * 0.25, 0.5 * 0.25f, -1.5f * 0.25);
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

			view = glm::lookAt(cameraPos, cameraFront, cameraUp);

			model = glm::translate(model, glm::vec3(-1.5f * 0.25, 0.5 * 0.25f, -1.5f * 0.25));
		}


		if (tag == 1)
		{
			ImGui::SliderFloat("para1", &para1, -1.0f, 0.0f);
			ImGui::SliderFloat("para2", &para2, 0.0f, 1.0f);
			ImGui::SliderFloat("para3", &para3, -1.0f, 0.0f);
			ImGui::SliderFloat("para4", &para4, 0.0f, 1.0f);
			ImGui::SliderFloat("distance", &distance, 0.1f, 100.0f);
			projection = glm::ortho(para1, para2, para3, para4, 0.1f, distance);
			float camPosX = 3.0f;
			float camPosY = 3.0f;
			float camPosZ = 3.0f;
			glm::vec3 cameraPos = glm::vec3(camPosX, camPosY, camPosZ);
			glm::vec3 cameraFront = glm::vec3(-1.5f * 0.25, 0.5 * 0.25f, -1.5f * 0.25);
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

			view = glm::lookAt(cameraPos, cameraFront, cameraUp);

			model = glm::translate(model, glm::vec3(-1.5f * 0.25, 0.5 * 0.25f, -1.5f * 0.25));
		}
		else if (tag == 2)
		{
			ImGui::SliderFloat("locX", &locX, -5.0f, 5.0f);
			ImGui::SliderFloat("locY", &locY, -5.0f, 5.0f);
			ImGui::SliderFloat("locZ", &locZ, -5.0f, 5.0f);
			float radius = sqrt(locX * locX + locY * locY + locZ * locZ);
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			float camPosX = locX;
			float camPosY = locY;
			float camPosZ = locZ;
			camPosX = sin(clock() / 1000.0) * radius; camPosZ = cos(clock() / 1000.0) * radius;
			glm::vec3 cameraPos = glm::vec3(camPosX, camPosY, camPosZ);
			glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

			view = glm::lookAt(cameraPos, cameraFront, cameraUp);

			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		}

		else if (tag == 3)
		{
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		}
		*/

		if (lightflag) {
			lightPosition = glm::vec3(3 * sin(glfwGetTime()) * 1.0f + 1.0f, 1.0f, 3 * cos(glfwGetTime()) * 1.0f);
		}

		glm::mat4 lightmodel = glm::mat4(1.0f);
		lightmodel = glm::translate(lightmodel, lightPosition);
		lightmodel = glm::scale(lightmodel, glm::vec3(0.2f));

		glUseProgram(shaderProgramLight);

		unsigned int lightmodelLoc = glGetUniformLocation(shaderProgramLight, "model");
		unsigned int lightVLoc = glGetUniformLocation(shaderProgramLight, "view");
		unsigned int lightPLoc = glGetUniformLocation(shaderProgramLight, "projection");
		glUniformMatrix4fv(lightmodelLoc, 1, GL_FALSE, glm::value_ptr(lightmodel));
		glUniformMatrix4fv(lightVLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(lightPLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(LightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		if (flag) {
			glUseProgram(shaderProgramPhong);

			unsigned int Vloc = glGetUniformLocation(shaderProgramPhong, "view");
			glUniformMatrix4fv(Vloc, 1, GL_FALSE, glm::value_ptr(view));
			unsigned int Ploc = glGetUniformLocation(shaderProgramPhong, "projection");
			glUniformMatrix4fv(Ploc, 1, GL_FALSE, glm::value_ptr(projection));
			unsigned int modelLoc = glGetUniformLocation(shaderProgramPhong, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			unsigned int viewPosLoc = glGetUniformLocation(shaderProgramPhong, "viewPos");
			glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
			unsigned int lightPosLoc = glGetUniformLocation(shaderProgramPhong, "lightPos");
			glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPosition));
			unsigned int objectColorLoc = glGetUniformLocation(shaderProgramPhong, "objectColor");
			glUniform3fv(objectColorLoc, 1, glm::value_ptr(glm::vec3(0.8f, 0.5f, 0.6f)));
			unsigned int lightColorLoc = glGetUniformLocation(shaderProgramPhong, "lightColor");
			glUniform3fv(lightColorLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

			unsigned int ambientLoc = glGetUniformLocation(shaderProgramPhong, "ambientFactor");
			glUniform1fv(ambientLoc, 1, &ambientFactor);
			unsigned int diffuseLoc = glGetUniformLocation(shaderProgramPhong, "diffuseFactor");
			glUniform1fv(diffuseLoc, 1, &diffuseFactor);
			unsigned int specularLoc = glGetUniformLocation(shaderProgramPhong, "specularFactor");
			glUniform1fv(specularLoc, 1, &specularFactor);
			unsigned int shininessLoc = glGetUniformLocation(shaderProgramPhong, "shininessFactor");
			glUniform1iv(shininessLoc, 1, &shininessFactor);


		}
		else {

			glUseProgram(shaderProgramGouraud);
			unsigned int Vloc = glGetUniformLocation(shaderProgramGouraud, "view");
			glUniformMatrix4fv(Vloc, 1, GL_FALSE, glm::value_ptr(view));
			unsigned int Ploc = glGetUniformLocation(shaderProgramGouraud, "projection");
			glUniformMatrix4fv(Ploc, 1, GL_FALSE, glm::value_ptr(projection));
			unsigned int modelLoc = glGetUniformLocation(shaderProgramGouraud, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			unsigned int viewPosLoc = glGetUniformLocation(shaderProgramGouraud, "viewPos");
			glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
			unsigned int lightPosLoc = glGetUniformLocation(shaderProgramGouraud, "lightPos");
			glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPosition));
			unsigned int objectColorLoc = glGetUniformLocation(shaderProgramGouraud, "objectColor");
			glUniform3fv(objectColorLoc, 1, glm::value_ptr(glm::vec3(0.8f, 0.5f, 0.6f)));
			unsigned int lightColorLoc = glGetUniformLocation(shaderProgramGouraud, "lightColor");
			glUniform3fv(lightColorLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

			unsigned int ambientLoc = glGetUniformLocation(shaderProgramGouraud, "ambientFactor");
			glUniform1fv(ambientLoc, 1, &ambientFactor);
			unsigned int diffuseLoc = glGetUniformLocation(shaderProgramGouraud, "diffuseFactor");
			glUniform1fv(diffuseLoc, 1, &diffuseFactor);
			unsigned int specularLoc = glGetUniformLocation(shaderProgramGouraud, "specularFactor");
			glUniform1fv(specularLoc, 1, &specularFactor);
			unsigned int shininessLoc = glGetUniformLocation(shaderProgramGouraud, "shininessFactor");
			glUniform1iv(shininessLoc, 1, &shininessFactor);

		}


		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
