#include <iostream>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <irrKlang/irrKlang.h>

#include "Shader.h"
#include "RubiksCube.h"
#include "CameraController.h"

GLFWwindow *window;

const GLuint screenWidth = 1600;
const GLuint screenHeight = 900;

GLuint rubiksCubeTexture;
GLuint blanktTileTexture;
GLuint skyboxTextures;

GLuint rubiksCubeVAO;
GLuint rubiksCubeVBO;
GLuint skyboxVAO;
GLuint skyboxVBO;

Shader *lightingShader = nullptr;
Shader *skyboxShader = nullptr;

CameraController camera(glm::vec3(0, 0, 0));

std::vector<TurnMove*> currentMove = {};
std::vector<std::array<int, 5>> moveQueue = {};

using namespace irrklang;
ISoundEngine* SoundEngine = createIrrKlangDevice();

bool freeCam = false;
bool firstMouseInput = true;
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;

int rubiksCubeXAxis = XAXIS;
int rubiksCubeYAxis = YAXIS;
int rubiksCubeZAxis = ZAXIS;

int topFace = WHITEFACE;
int frontFace = BLUEFACE;

float deltaTime = 0.0f;
float animationAngle = 0.0f;

bool wKeyPressed = false;
bool aKeyPressed = false;
bool sKeyPressed = false;
bool dKeyPressed = false;
bool qKeyPressed = false;
bool eKeyPressed = false;

bool upArrowPressed = false;
bool leftArrowPressed = false;
bool downArrowPressed = false;
bool rightArrowPressed = false;

bool fKeyPressed = false;

class Cubie
{
public:

	std::vector<glm::vec3> colors;
	glm::vec3 cubiePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	std::array<bool, 6> visible = {};
	std::vector<Cubie*> cubies = {};
	glm::mat4 cubieTransformationMatrix = glm::mat4(1.0f);


	Cubie(std::vector<glm::vec3> colors, std::array<bool, 6> visible, glm::vec3 cubiePosition)
	{
		this->colors = colors;
		this->cubiePosition = cubiePosition;
		this->visible = visible;
		this->cubieTransformationMatrix = glm::translate(this->cubieTransformationMatrix, this->cubiePosition);
	}

	void DrawCubie(Shader* lightingShader)
	{
		float diffuseMultiplier = 1.50f;
		float ambientMultiplier = 1.0f * diffuseMultiplier;

		lightingShader->Use();

		glm::mat4 lightingShaderModel = glm::mat4(1.0f);
		lightingShaderModel *= this->cubieTransformationMatrix;

		lightingShader->SetMatrix3("normal", glm::inverse(lightingShaderModel));
		lightingShader->SetMatrix4("model", lightingShaderModel);

		for (int i = 0; i < 6; ++i)
		{
			if (visible[i] == true)
			{
				glBindTexture(GL_TEXTURE_2D, rubiksCubeTexture);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, blanktTileTexture);
			}

			lightingShader->SetVector3("material.ambient", this->colors[i][0] * ambientMultiplier, this->colors[i][1] * ambientMultiplier, this->colors[i][2] * ambientMultiplier);
			lightingShader->SetVector3("material.diffuse", this->colors[i][0] * diffuseMultiplier, this->colors[i][1] * diffuseMultiplier, this->colors[i][2] * diffuseMultiplier);

			glDrawArrays(GL_TRIANGLES, 6 * i, 6);
		}

		for (int i = 0; i < cubies.size(); ++i)
		{
			cubies[i]->DrawCubie(lightingShader);
		}
	}

	void Rotate(glm::mat4 transformationMatrix, float rotationAngle, glm::vec3 cubieAxis, glm::vec3 worldAxis)
	{
		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::mat4 rotationQuaternion = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), worldAxis);
		glm::mat4 inverseRotationQuaternion = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), -worldAxis);

		this->cubieTransformationMatrix = glm::rotate(transformationMatrix, glm::radians(rotationAngle), cubieAxis);

		for (int i = 0; i < cubies.size(); ++i)
		{
			glm::mat4 transformationMatrix = rotationQuaternion * cubies[i]->cubieTransformationMatrix * inverseRotationQuaternion;

			cubies[i]->Rotate(transformationMatrix, rotationAngle, worldAxis, worldAxis);

			glm::decompose(cubies[i]->cubieTransformationMatrix, scale, orientation, cubies[i]->cubiePosition, skew, perspective);
		}
	}

	void AddCubie(std::vector<glm::vec3> colors, std::array<bool, 6> visible, glm::vec3 position)
	{
		cubies.push_back(new Cubie(colors, visible, position));
	}

	void CopyCubie(Cubie& cubieToCopy)
	{
		cubies.push_back(&cubieToCopy);
	}

	void DeleteCubie(Cubie& cubieToRemove)
	{
		for (int i = 0; i < cubies.size(); ++i)
		{
			if (cubies[i] == &cubieToRemove)
			{
				cubies.erase(cubies.begin() + i);

				break;
			}
		}
	}

	void GetPosition(std::vector<glm::vec3> *positions)
	{
		positions->push_back(this->cubiePosition);
		for (int i = 0; i < cubies.size(); ++i)
		{
			cubies[i]->GetPosition(positions);
		}
	}
};

int main()
{
	if (!InitialiseOpenGL())
	{
		return -1;
	}

	InitialiseCamera();

	lightingShader = new Shader("Shaders/LightingShader.vert", "Shaders/LightingShader.frag");
	skyboxShader = new Shader("Shaders/SkyboxShader.vert", "Shaders/SkyboxShader.frag");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(screenWidth), 0.0f, static_cast<GLfloat>(screenHeight));

	BuildRubiksCube();
	BindVertexData();
	LoadTextures();

	PrintInscructions();

	float oldTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		float time = glfwGetTime();
		deltaTime = (time - oldTime) * 1000;
		oldTime = time;

		ProcessKeyboard(window);
		Update();
		Draw();
	}

	ClearResources();
	return 0;
}

bool InitialiseOpenGL()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(screenWidth, screenHeight, "Rubik's Cube", nullptr, nullptr);
	if (window == NULL)
	{
		std::cerr << "ERROR: Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, CursorPosCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "ERROR: Failed to initialize the GLAD Library" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	//glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_STENCIL_TEST);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void InitialiseLighting(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	lightingShader->SetVector3("dirLight.direction", lightDirection);
	lightingShader->SetVector3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
	lightingShader->SetVector3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	lightingShader->SetVector3("dirLight.specular", 0.6f, 0.6f, 0.6f);

	for (int i = 0; i < (sizeof(pointLightPosition) / sizeof(*pointLightPosition)); ++i)
	{
		lightingShader->SetVector3("pointLights[" + std::to_string(i) + "].position", pointLightPosition[i]);
		lightingShader->SetVector3("pointLights[" + std::to_string(i) + "].ambient", 0.3f, 0.3f, 0.3f);
		lightingShader->SetVector3("pointLights[" + std::to_string(i) + "].diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader->SetVector3("pointLights[" + std::to_string(i) + "].specular", 0.6f, 0.6f, 0.6f);

		lightingShader->SetFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
		lightingShader->SetFloat("pointLights[" + std::to_string(i) + "].linear", 0.9f);
		lightingShader->SetFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.03f);
	}

	lightingShader->SetVector3("viewPos", camera.cameraPosition);
	lightingShader->SetVector3("material.specular", 0.6f, 0.6f, 0.6f);

	lightingShader->SetFloat("material.shininess", 32.0f);

	lightingShader->SetMatrix4("projection", projection);
	lightingShader->SetMatrix4("view", view);
	lightingShader->SetMatrix4("model", model);
}

void InitialiseCamera()
{
	camera.cameraPosition = glm::vec3(8.6f, 6.8f, 13.0f);
	camera.cameraYaw = -120.1f;
	camera.cameraPitch = -26.6f;
	camera.UpdateCameraVectors();
}

void BindVertexData()
{
	glGenVertexArrays(1, &rubiksCubeVAO);
	glGenBuffers(1, &rubiksCubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, rubiksCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, rubiksCubeVertexData.size() * sizeof(float), &rubiksCubeVertexData[0], GL_STATIC_DRAW);

	glBindVertexArray(rubiksCubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxVertexData.size() * sizeof(GLfloat), &skyboxVertexData[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
}

void LoadTextures()
{
	rubiksCubeTexture = LoadTexture("cubieTexture", { "Resources/Textures/RubiksCubeTile.png" });
	blanktTileTexture = LoadTexture("cubieTexture", { "Resources/Textures/BlackTile.png" });

	lightingShader->Use();
	lightingShader->SetInt("material.diffuseMap", 0);

	std::vector<std::string> skyboxFaces
	{
		"Resources/Skybox/SkyboxFront.png",
		"Resources/Skybox/SkyboxBack.png",
		"Resources/Skybox/SkyboxTopRotated.png",
		"Resources/Skybox/SkyboxBottomRotated.png",
		"Resources/Skybox/SkyboxRight.png",
		"Resources/Skybox/SkyboxLeft.png",
	};

	skyboxTextures = LoadTexture("skyboxTexture", skyboxFaces);

	skyboxShader->Use();
	skyboxShader->SetInt("skybox", 0);
}

Cubie *rubiksCube = new Cubie({				{black}, {black}, {black}, {black}, {black}, {black} },		{ false, false, false, false, false, false },	glm::vec3(0.0f, 0.0f, 0.0f));

void BuildRubiksCube()
{
	rubiksCube->AddCubie({					{white}, {black}, {black}, {black}, {black}, {black} },		{ true, false, false, false, false, false },	glm::vec3(0.0f, 2.1f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {black}, {blue}, {red}, {black}, {black} },		{ true, false, true, true, false, false },		glm::vec3(-2.1f, 2.1f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {black}, {black}, {red}, {black}, {black} },		{ true, false, false, true, false, false },		glm::vec3(-2.1f, 2.1f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {black}, {black}, {red}, {green}, {black} },		{ true, false, false, true, true, false },		glm::vec3(-2.1f, 2.1f, -2.1f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {black}, {blue}, {black}, {black}, {black} },		{ true, false, true, false, false, false },		glm::vec3(0.0f, 2.1f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {black}, {black}, {black}, {green}, {black} },		{ true, false, false, false, true, false },		glm::vec3(0.0f, 2.1f, -2.1f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {orange}, {blue}, {black}, {black}, {black} },		{ true, true, true, false, false, false },		glm::vec3(2.1f, 2.1f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {orange}, {black}, {black}, {black}, {black} },	{ true, true, false, false, false },			glm::vec3(2.1f, 2.1f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{white}, {orange}, {black}, {black}, {green}, {black} },	{ true, true, false, false, true, false },		glm::vec3(2.1f, 2.1f, -2.1f));

	rubiksCube->AddCubie({					{black}, {orange}, {black}, {black}, {black}, {black} },	{ false, true, false, false, false, false },	glm::vec3(2.1f, 0.0f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {orange}, {blue}, {black}, {black}, {black} },		{ false, true, true, false, false, false },		glm::vec3(2.1f, 0.0f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {orange}, {black}, {black}, {green}, {black} },	{ false, true, false, false, true, false },		glm::vec3(2.1f, 0.0f, -2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {orange}, {blue}, {black}, {black}, {yellow} },	{ false, true, true, false, false, true },		glm::vec3(2.1f, -2.1f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {orange}, {black}, {black}, {black}, {yellow} },	{ false, true, false, false, false, true },		glm::vec3(2.1f, -2.1f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {orange}, {black}, {black}, {green}, {yellow} },	{ false, true, false, false, true, true },		glm::vec3(2.1f, -2.1f, -2.1f));

	rubiksCube->AddCubie({					{black}, {black}, {blue}, {black}, {black}, {black} },		{ false, false, true, false, false, false },	glm::vec3(0.0f, 0.0f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {blue}, {red}, {black}, {black} },		{ false, false, true, true, false, false },		glm::vec3(-2.1f, 0.0f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {blue}, {red}, {black}, {yellow} },		{ false, false, true, true, false, true },		glm::vec3(-2.1f, -2.1f, 2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {blue}, {black}, {black}, {yellow} },		{ false, false, true, false, false, true },		glm::vec3(0.0f, -2.1f, 2.1f));

	rubiksCube->AddCubie({					{black}, {black}, {black}, {red}, {black}, {black} },		{ false, false, false, true, false, false },	glm::vec3(-2.1f, 0.0f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {black}, {red}, {green}, {black} },		{ false, false, false, true, true, false },		glm::vec3(-2.1f, 0.0f, -2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {black}, {red}, {black}, {yellow} },		{ false, false, false, true, false, true },		glm::vec3(-2.1f, -2.1f, 0.0f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {black}, {red}, {green}, {yellow} },		{ false, false, false, true, true, true },		glm::vec3(-2.1f, -2.1f, -2.1f));
		
	rubiksCube->AddCubie({					{black}, {black}, {black}, {black}, {green}, {black} },		{ false, false, false, false, true, false },	glm::vec3(0.0f, 0.0f, -2.1f));
	rubiksCube->cubies.back()->AddCubie({	{black}, {black}, {black}, {black}, {green}, {yellow} },	{ false, false, false, false, true, true },		glm::vec3(0.0f, -2.1f, -2.1f));

	rubiksCube->AddCubie({					{black}, {black}, {black}, {black}, {black}, {yellow} },	{ false, false, false, false, false, true },	glm::vec3(0.0f, -2.1f, 0.0f));
}

void Draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.cameraZoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	DrawCube(lightingShader, projection, view, model);
	DrawSkybox(projection, view, model);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void DrawCube(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	lightingShader->Use();

	InitialiseLighting(lightingShader, projection, view, model);

	glBindVertexArray(rubiksCubeVAO);
	rubiksCube->DrawCubie(lightingShader);

}

void DrawSkybox(glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	glDepthMask(GL_FALSE);

	skyboxShader->Use();
	
	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	skyboxShader->SetMatrix4("view", view);
	skyboxShader->SetMatrix4("projection", projection);

	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextures);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

void ProcessKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (currentMove.empty() || moveQueue.empty())
	{
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			fKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && fKeyPressed)
		{
			if (freeCam)
			{
				SoundEngine->play2D("Resources/Sounds/SwitchSound.mp3");
				//std::cout << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << ", " << camera.Yaw << ", " << camera.Pitch << std::endl;
				std::cout << "FreeCam deactivated" << std::endl;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				InitialiseCamera();

				freeCam = false;
			}
			else
			{
				SoundEngine->play2D("Resources/Sounds/SwitchSound.mp3");
				std::cout << "FreeCam activated" << std::endl;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				freeCam = true;
			}
			fKeyPressed = false;
		}
	}

	if (!freeCam)
	{
		if (!currentMove.empty() || !moveQueue.empty())
		{
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				upArrowPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && upArrowPressed)
			{
				RotateRubiksCube(PITCHUP);

				upArrowPressed = false;
			}

			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				downArrowPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE && downArrowPressed)
			{
				RotateRubiksCube(PITCHDOWN);

				downArrowPressed = false;
			}

			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				leftArrowPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE && leftArrowPressed)
			{
				RotateRubiksCube(YAWLEFT);

				leftArrowPressed = false;
			}

			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				rightArrowPressed = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && rightArrowPressed)
			{
				RotateRubiksCube(YAWRIGHT);

				rightArrowPressed = false;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			wKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && wKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNTOP, -1);
			}
			else
			{
				EnqueueTurnMove(TURNTOP, 1);
			}

			wKeyPressed = false;
		}

		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && wKeyPressed == false)
		{
			aKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && aKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNLEFT, -1);
			}
			else
			{
				EnqueueTurnMove(TURNLEFT, 1);
			}

			aKeyPressed = false;
		}

		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			sKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && sKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNBOTTOM, -1);
			}
			else
			{
				EnqueueTurnMove(TURNBOTTOM, 1);
			}

			sKeyPressed = false;
		}

		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			dKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && dKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNRIGHT, -1);
			}
			else
			{
				EnqueueTurnMove(TURNRIGHT, 1);
			}

			dKeyPressed = false;
		}

		else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			qKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE && qKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNFRONT, -1);
			}
			else
			{
				EnqueueTurnMove(TURNFRONT, 1);
			}

			qKeyPressed = false;
		}

		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			eKeyPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && eKeyPressed)
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				EnqueueTurnMove(TURNBACK, -1);
			}
			else
			{

				EnqueueTurnMove(TURNBACK, 1);

			}

			eKeyPressed = false;
		}
	}

	else if (freeCam)
	{
		if (glfwGetKey(window, GLFW_KEY_W))
		{
			camera.ProcessKeyboard(FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			camera.ProcessKeyboard(LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			camera.ProcessKeyboard(RIGHT, deltaTime);
		}
	}
}

void RotateRubiksCube(int moveType)
{
	int newFrontFace = 0;
	int newTopFace = 0;

	if (faceLookupTable[frontFace][moveType][topFace][0] == 'W')
		newFrontFace = YELLOWFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][0] == 'O')
		newFrontFace = REDFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][0] == 'B')
		newFrontFace = GREENFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][0] == 'R')
		newFrontFace = ORANGEFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][0] == 'G')
		newFrontFace = BLUEFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][0] == 'Y')
		newFrontFace = WHITEFACE;

	if (faceLookupTable[frontFace][moveType][topFace][1] == 'W')
		newTopFace = YELLOWFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][1] == 'O')
		newTopFace = REDFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][1] == 'B')
		newTopFace = GREENFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][1] == 'R')
		newTopFace = ORANGEFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][1] == 'G')
		newTopFace = BLUEFACE;
	else if (faceLookupTable[frontFace][moveType][topFace][1] == 'Y')
		newTopFace = WHITEFACE;

	frontFace = newFrontFace;
	topFace = newTopFace;

	switch (frontFace)
	{
	case WHITEFACE:
	case YELLOWFACE:
		switch (topFace)
		{
		case REDFACE:
		case ORANGEFACE:
			rubiksCubeXAxis = ZAXIS;
			rubiksCubeYAxis = XAXIS;
			break;
		case BLUEFACE:
		case GREENFACE:
			rubiksCubeXAxis = XAXIS;
			rubiksCubeYAxis = ZAXIS;
			break;
		}
		rubiksCubeZAxis = YAXIS;
		break;
	case BLUEFACE:
	case GREENFACE:
		switch (topFace)
		{
		case ORANGEFACE:
		case REDFACE:
			rubiksCubeXAxis = YAXIS;
			rubiksCubeYAxis = XAXIS;
			break;
		case YELLOWFACE:
		case WHITEFACE:
			rubiksCubeXAxis = XAXIS;
			rubiksCubeYAxis = YAXIS;
			break;
		}
		rubiksCubeZAxis = ZAXIS;
		break;
	case ORANGEFACE:
	case REDFACE:
		switch (topFace)
		{
		case BLUEFACE:
		case GREENFACE:
			rubiksCubeXAxis = YAXIS;
			rubiksCubeYAxis = ZAXIS;
			break;
		case WHITEFACE:
		case YELLOWFACE:
			rubiksCubeXAxis = ZAXIS;
			rubiksCubeYAxis = YAXIS;
			break;
		}
		rubiksCubeZAxis = XAXIS;
		break;
	}

	SoundEngine->play2D("Resources/Sounds/SwooshSound.mp3");

	switch (moveType) 
	{
	case PITCHUP:
		currentMove.push_back(new TurnMove(0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false));
		break;
	case PITCHDOWN:
		currentMove.push_back(new TurnMove(0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, false));
		break;
	case YAWLEFT:
		currentMove.push_back(new TurnMove(0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, false));
		break;
	case YAWRIGHT:
		currentMove.push_back(new TurnMove(0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, false));
		break;
	}
}

void Update()
{
	if (currentMove.empty() && !moveQueue.empty())
	{
		PerformTurnMove(moveQueue.back()[0], moveQueue.back()[1], moveQueue.back()[2]);

		moveQueue.pop_back();
	}

	if (!currentMove.empty())
	{
		animationAngle += (currentMove[0]->rotationAngle / 200) * deltaTime; 

		if (currentMove[0]->cubie)
		{
			rubiksCube->cubies[currentMove[0]->faceIndex]->Rotate(rubiksCube->cubies[currentMove[0]->faceIndex]->cubieTransformationMatrix, animationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
			rubiksCube->cubies[currentMove[0]->faceIndex]->Rotate(rubiksCube->cubies[currentMove[0]->faceIndex]->cubieTransformationMatrix, -currentMove[0]->currentRotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
		}
		else
		{
			rubiksCube->Rotate(rubiksCube->cubieTransformationMatrix, animationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
			rubiksCube->Rotate(rubiksCube->cubieTransformationMatrix, -currentMove[0]->currentRotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
		}

		currentMove[0]->currentRotationAngle = animationAngle;

		if (sqrtf(animationAngle * animationAngle) >= sqrtf(currentMove[0]->rotationAngle * currentMove[0]->rotationAngle))
		{
			if (currentMove[0]->cubie)
			{
				rubiksCube->cubies[currentMove[0]->faceIndex]->Rotate(rubiksCube->cubies[currentMove[0]->faceIndex]->cubieTransformationMatrix, currentMove[0]->rotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
				rubiksCube->cubies[currentMove[0]->faceIndex]->Rotate(rubiksCube->cubies[currentMove[0]->faceIndex]->cubieTransformationMatrix, -currentMove[0]->currentRotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
			}
			else
			{
				rubiksCube->Rotate(rubiksCube->cubieTransformationMatrix, currentMove[0]->rotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
				rubiksCube->Rotate(rubiksCube->cubieTransformationMatrix, -currentMove[0]->currentRotationAngle, currentMove[0]->cubieAxis, currentMove[0]->worldAxis);
			}
			
			animationAngle = 0.0f;

			currentMove.pop_back();
		}
	}
}

void EnqueueTurnMove(int moveType, int moveDirection)
{
	int faceIndex = 0;

	for (int i = 0; i < rubiksCube->cubies.size(); ++i)
	{
		switch (moveType)
		{
		case TURNTOP:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.y, 2.1f))
			{
				faceIndex = i;
			}
			break;
		case TURNRIGHT:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.x, 2.1f)) 
			{ 
				faceIndex = i; 
			}
			break;
		case TURNFRONT:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.z, 2.1f))
			{
				faceIndex = i;
			}
			break;
		case TURNLEFT:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.x, -2.1f))
			{
				faceIndex = i;
			}
			break;
		case TURNBACK:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.z, -2.1f))
			{
				faceIndex = i;
			}
			break;
		case TURNBOTTOM:
			if (ComparePositions(rubiksCube->cubies[i]->cubiePosition.y, -2.1f))
			{
				faceIndex = i;
			}
			break;
		}
	}
	moveQueue.insert(moveQueue.begin(), { faceIndex, moveDirection, moveType });
}

void AttachRingCubies(int faceIndex, std::vector<glm::vec3> positions, std::vector<int> cubePositions)
{
	if (cubePositions.size() > 0)
	{
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < rubiksCube->cubies.size(); ++j)
			{
				if (j != faceIndex)
				{
					for (int k = 0; k < rubiksCube->cubies[j]->cubies.size(); ++k)
					{
						if (ComparePositions(positions[cubePositions[i]].x, rubiksCube->cubies[j]->cubies[k]->cubiePosition.x) && ComparePositions(positions[cubePositions[i]].y, rubiksCube->cubies[j]->cubies[k]->cubiePosition.y) && ComparePositions(positions[cubePositions[i]].z, rubiksCube->cubies[j]->cubies[k]->cubiePosition.z))
						{
							rubiksCube->cubies[faceIndex]->CopyCubie(*rubiksCube->cubies[j]->cubies[k]);

							rubiksCube->cubies[j]->DeleteCubie(*rubiksCube->cubies[j]->cubies[k]);
						}
					}
				}
			}
		}
	}
}

void PerformTurnMove(int faceIndex, int moveDirection, int moveType) 
{
	std::vector<glm::vec3> positions = {};

	std::vector<int> cubiesToAttach = {};

	glm::vec3 axis;

	rubiksCube->GetPosition(&positions);

	SoundEngine->play2D("Resources/Sounds/TurnSound.mp3");

	switch(moveType)
	{
		case TURNTOP:
			axis = GetAxis(rubiksCubeYAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].y, rubiksCube->cubies[faceIndex]->cubiePosition.y))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f));
			}
			break;
		case TURNBOTTOM:
			axis = GetAxis(rubiksCubeYAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].y, rubiksCube->cubies[faceIndex]->cubiePosition.y))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f));
			}
			break;
		case TURNLEFT:
			axis = GetAxis(rubiksCubeXAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].x, rubiksCube->cubies[faceIndex]->cubiePosition.x))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), 90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), -90.0f));
			}
			break;
		case TURNRIGHT:
			axis = GetAxis(rubiksCubeXAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].x, rubiksCube->cubies[faceIndex]->cubiePosition.x))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), -90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(1.0f, 0.0f, 0.0f), 90.0f));
			}
			break;
		case TURNFRONT:
			axis = GetAxis(rubiksCubeZAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].z, rubiksCube->cubies[faceIndex]->cubiePosition.z))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), -90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), 90.0f));
			}
			break;
		case TURNBACK:
			axis = GetAxis(rubiksCubeZAxis, faceIndex);

			for (int i = 0; i < positions.size(); ++i)
			{
				if (positions[i] != rubiksCube->cubies[faceIndex]->cubiePosition && ComparePositions(positions[i].z, rubiksCube->cubies[faceIndex]->cubiePosition.z))
				{
					cubiesToAttach.push_back(i);
				}
			}

			AttachRingCubies(faceIndex, positions, cubiesToAttach);

			if (moveDirection == 1)
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), 90.0f));
			}
			else
			{
				currentMove.push_back(new TurnMove(faceIndex, axis, glm::vec3(0.0f, 0.0f, 1.0f), -90.0f));
			}
			break;
	}
}

void CursorPosCallback(GLFWwindow *window, double x, double y)
{
	if (freeCam)
	{
		if (firstMouseInput)
		{
			lastX = x;
			lastY = y;
			firstMouseInput = false;
		}

		float xOffset = x - lastX;
		float yOffset = lastY - y;

		lastX = x;
		lastY = y;

		camera.ProcessMouse(xOffset, yOffset);
	}
}

void ClearResources()
{
	for (int i = 0; i < currentMove.size(); ++i)
	{
		delete currentMove[i];
	}

	delete lightingShader;
	delete skyboxShader;

	SoundEngine->drop();
	
	glDeleteVertexArrays(1, &rubiksCubeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);

	glDeleteBuffers(1, &rubiksCubeVBO);
	glDeleteBuffers(1, &skyboxVBO);
	
	glfwDestroyWindow(window);
	glfwTerminate();	

	exit(0);
}