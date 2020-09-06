#pragma once

#define YAW 0.0f
#define PITCH 0.0f
#define ROLL 0.0f
#define SPEED 20.0f
#define SENSITIVITY 0.2f
#define ZOOM 70.0f

enum cameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

class CameraController
{
public:

	glm::vec3 cameraPosition;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	glm::vec3 cameraWorldUp;

	float cameraYaw;
	float cameraPitch;
	float cameraRoll;

	float cameraMovementSpeed;
	float cameraSensitivity;
	float cameraZoom;

	CameraController(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = ROLL)
	{
		cameraPosition = position;

		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

		cameraMovementSpeed = SPEED;
		cameraSensitivity = SENSITIVITY;

		cameraZoom = ZOOM;

		cameraWorldUp = up;

		cameraYaw = yaw;
		cameraPitch = pitch;
		cameraRoll = roll;

		UpdateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
	}

	void ProcessKeyboard(cameraMovement direction, float deltaTime)
	{
		float velocity = cameraMovementSpeed * (deltaTime / 1000);

		if (direction == FORWARD)
		{
			cameraPosition += cameraFront * velocity;
		}

		if (direction == BACKWARD)
		{
			cameraPosition -= cameraFront * velocity;
		}

		if (direction == LEFT)
		{
			cameraPosition -= cameraRight * velocity;
		}

		if (direction == RIGHT)
		{
			cameraPosition += cameraRight * velocity;
		}
	}

	void UpdateCameraVectors()
	{
		glm::vec3 front;

		front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		front.y = sin(glm::radians(cameraPitch));
		front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

		cameraFront = glm::normalize(front);

		cameraRight = glm::normalize(glm::cross(cameraFront, cameraWorldUp));

		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
	}

	void ProcessMouse(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= cameraSensitivity;
		yoffset *= cameraSensitivity;

		cameraYaw += xoffset;
		cameraPitch += yoffset;

		if (constrainPitch)
		{
			if (cameraPitch > 89.0f)
			{
				cameraPitch = 89.0f;
			}

			if (cameraPitch < -89.0f)
			{
				cameraPitch = -89.0f;
			}
		}
		UpdateCameraVectors();
	}	
};