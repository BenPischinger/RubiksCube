#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <stb_image.h>

enum Axes { XAXIS, YAXIS, ZAXIS };
enum Faces { WHITEFACE, ORANGEFACE, BLUEFACE, REDFACE, GREENFACE, YELLOWFACE };
enum Moves { YAWLEFT, YAWRIGHT, PITCHUP, PITCHDOWN, TURNTOP, TURNRIGHT, TURNFRONT, TURNLEFT, TURNBACK, TURNBOTTOM, TURNMOVE, ROTATEMOVE, NONE};

int main();
bool InitialiseOpenGL();
void InitialiseLighting(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void InitialiseCamera();

void LoadTextures();
void BindVertexData();
void BuildRubiksCube();

void Draw();
void DrawSkybox(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
void DrawCube(Shader* lightingShader, glm::mat4 projection, glm::mat4 view, glm::mat4 model);

void ProcessKeyboard(GLFWwindow *window);

void Update();

void RotateRubiksCube(int moveType);
void EnqueueTurnMove(int moveType, int moveDirection);
void AttachRingCubies(int faceIndex, std::vector<glm::vec3> positions, std::vector<int> cubePositions);
void PerformTurnMove(int faceIndex, int moveDirection, int moveType);

void CursorPosCallback(GLFWwindow *window, double x, double y);

void ClearResources();

glm::vec3 GetAxis(int axis, int faceIndex);

// Colours for the cube textures
glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 orange = glm::vec3(1.0f, 0.3f, 0.0f);
glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 green = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);

// Determines which direction the light of the lighting shader comes from
// Currently comes from the big star on the skybox
static glm::vec3 lightDirection = glm::vec3(9.0f, 13.0f, 7.0f);

// Positions of the point light surrounding the cube
static glm::vec3 pointLightPosition[] =
{
	glm::vec3(5.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 5.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 5.0f),
	glm::vec3(-5.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, -5.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, -5.0f),
};

// Geometrical data cube 
static std::vector<float>rubiksCubeVertexData = 
{
	-1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  1.0,
	 1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  0.0,
	 1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
	 1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  0.0,
	-1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  1.0,
	-1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  0.0,  0.0,

	 1.0,  1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  0.0,
	 1.0, -1.0, -1.0,  1.0,  0.0,  0.0,  0.0,  1.0,
	 1.0,  1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,
	 1.0, -1.0, -1.0,  1.0,  0.0,  0.0,  0.0,  1.0,
	 1.0,  1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  0.0,
	 1.0, -1.0,  1.0,  1.0,  0.0,  0.0,  0.0,  0.0,

	-1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,
	 1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,  0.0,
	 1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
	 1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  0.0,  1.0,
	-1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,

	-1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  0.0,
	-1.0,  1.0, -1.0, -1.0,  0.0,  0.0,  1.0,  1.0,
	-1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  1.0,
	-1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  1.0,
	-1.0, -1.0,  1.0, -1.0,  0.0,  0.0,  0.0,  0.0,
	-1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  0.0,

	-1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,
	 1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
	 1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  0.0,
	 1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
	-1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,
	-1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  1.0,

	-1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  1.0,
	 1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  1.0,  1.0,
	 1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0,
	 1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0,
	-1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  0.0,  0.0,
	-1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  1.0
};

// Geometrical data of the skybox
static std::vector<float>skyboxVertexData = 
{
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,	
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

// Lookup table to check which sides need to switch when the cube is being rotated 
static std::vector<std::vector<std::vector<std::string>>> faceLookupTable =
{
	{
		{"00", "GO", "OB", "BR", "BO", "00"},
		{"00", "BO", "RB", "GR", "OG", "00"},
		{"00", "OY", "BY", "RY", "GY", "00"},
		{"00", "RW", "GW", "OW", "BW", "00"}
	},

	{
		{"BW", "00", "YB", "00", "WG", "GY"},
		{"GW", "00", "WB", "00", "YG", "BY"},
		{"WR", "00", "BR", "00", "GR", "YR"},
		{"YO", "00", "GO", "00", "BO", "WO"}
	},

	{
		{"RW", "WO", "00", "YR", "00", "OY"},
		{"OW", "YO", "00", "WR", "00", "RY"},
		{"WG", "OG", "00", "RG", "00", "YG"},
		{"YB", "RB", "00", "OB", "00", "WB"}
	},

	{
		{"GW", "00", "WB", "00", "YG", "BY"},
		{"BW", "00", "YB", "00", "WG", "GY"},
		{"WO", "00", "BO", "00", "GO", "YR"},
		{"YR", "00", "GR", "00", "BR", "WR"}
	},

	{
		{"OW", "YO", "00", "WR", "00", "RY"},
		{"RW", "WO", "00", "YR", "00", "OY"},
		{"WB", "OB", "00", "RB", "00", "YB"},
		{"YG", "RG", "00", "OG", "00", "WG"}
	},

	{
		{"00", "BO", "RB", "GR", "OG", "00"},
		{"00", "GO", "OB", "BR", "RG", "00"},
		{"00", "OW", "BW", "RW", "GW", "00"},
		{"00", "RY", "GY", "OY", "BY", "00"}
	},
};

// Struct to define a move when turn a side of the cube
struct TurnMove
{
	int faceIndex = 0;

	glm::vec3 cubieAxis = glm::vec3(0.0f);
	glm::vec3 worldAxis = glm::vec3(0.0f);

	float rotationAngle = 0.0f;
	float currentRotationAngle = 0.0f;
	
	bool cubie = true;
	
	TurnMove(int face, glm::vec3 cubieAxis, glm::vec3 worldAxis, float rotationAngle, bool cubie = true)
	{
		this->faceIndex = face;

		this->cubieAxis = cubieAxis;
		this->worldAxis = worldAxis;
				
		this->rotationAngle = rotationAngle;
		this->currentRotationAngle = 0.0f;

		this->cubie = cubie;
	}
};

// Method to load the texture from the image files in the solution folder
static GLuint LoadTexture(std::string textureType, std::vector<std::string> filePath)
{
	GLuint texture;
	glGenTextures(1, &texture);

	int width; 
	int height; 
	int nrChannels;

	for (int i = 0; i < filePath.size(); i++)
	{
		unsigned char* data = stbi_load(filePath[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			if (textureType == "cubieTexture")
			{
				glBindTexture(GL_TEXTURE_2D, texture);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_RGBA);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_RGBA);
			}
			else if(textureType == "skyboxTexture")
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	return texture;
}

// Compares two positions and checks them against an accuracy value, due to vertices sometimes moving from 2.1 to 2.099999
bool ComparePositions(float a, float b)
{
	return std::abs(a - b) <= 0.1f;
}

// Returns the axis that the cube has to be rotated around
glm::vec3 GetAxis(int axisID, int faceIndex)
{
	glm::vec3 axis = glm::vec3(0, 0, 0);

	switch (axisID)
	{
	case XAXIS:
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case YAXIS:
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case ZAXIS:
		axis = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	}

	if (faceIndex != WHITEFACE && faceIndex != ORANGEFACE && faceIndex != BLUEFACE)
	{
		axis *= -1;
	}

	return axis;
}

// Self-explanatory, I hope
void PrintInscructions()
{
	std::cout << "Instructions: \n" << std::endl;
	std::cout << "Escape Key:	Close Game \n" << std::endl;
	std::cout << "Arrow Keys:	Rotate Cube \n" << std::endl;
	std::cout << "Shift Key:	Hold for Reverse Moves \n" << std::endl;
	std::cout << "W Key:		Turn Top" << std::endl;
	std::cout << "S Key:		Turn Bottom " << std::endl;
	std::cout << "A Key:		Turn Left" << std::endl;
	std::cout << "D Key:		Turn Right" << std::endl;
	std::cout << "Q Key:		Turn Front" << std::endl;
	std::cout << "E Key:		Turn Back \n" << std::endl;
	std::cout << "F Key:		Free Cam \n" << std::endl;
}
