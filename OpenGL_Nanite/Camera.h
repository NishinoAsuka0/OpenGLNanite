#pragma once
#include "OpenGLwin.h"

enum CameraMovement {
	FORWARD,		// 向前
	BACKWARD,		// 向后
	LEFT,			// 向左
	RIGHT,			// 向右
	UPWARD,			// 向上
	DOWNWARD		// 向下
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
private:
	// 摄影机的属性
	glm::vec3 position;	//位置
	glm::vec3 front;	//摄像机的Z轴
	glm::vec3 up;	//摄像机的Y轴
	glm::vec3 right;	//摄像机的X轴
	glm::vec3 worldUp;	//世界下的Y轴

	GLfloat movementSpeed;			// 镜头移动速度

	float yaw;		//俯仰角
	float pitch;		//偏移角
	float mouseSensitivity;	//鼠标灵敏度
	float zoom;		//投影深度

	void UpdateCameraVectors();

public:
	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		UpdateCameraVectors();
	}

	glm::mat4 GetViewMatrix();

	glm::vec3 GetPosition();

	//获得当前摄像机的视野
	GLfloat GetZoom();

	//键盘控制的视角变化
	void ProcessKeyboard(CameraMovement direction, float deltaTime);

	//鼠标移动的视角变化
	void ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch = true);

	//鼠标滚轮缩放
	void ProcessMouseScroll(float yOffset);
};
