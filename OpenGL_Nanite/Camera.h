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

const GLfloat SPEED = 6.0f;

class Camera {
private:
	// 摄影机的属性
	glm::vec3 position;				// 相机当前位置 
	glm::vec3 cameraZAxis;		// 摄影机的 Z 轴向量
	glm::vec3 cameraXAxis;		// 摄影机的 X 轴向量
	glm::vec3 cameraYAxis;		// 摄影机的 Y 轴向量
	GLfloat movementSpeed;			// 镜头移动速度

	float yaw;		//俯仰角
	float pitch;		//偏移角
	float mouseSensitivity;	//鼠标灵敏度
	float zoom;		//投影深度

	void UpdateCameraVectors();

public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) :movementSpeed(SPEED) {
		this->position = position;
		this->cameraZAxis = target;
		this->cameraYAxis = up;
		this->cameraXAxis = glm::normalize(glm::cross(this->cameraZAxis, this->cameraYAxis));

		this->zoom = 45.0f;
		this->yaw = 90.0f;
		this->pitch = 0.0f;
		this->mouseSensitivity = 0.2f;
		this->UpdateCameraVectors();		// 实时更新
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
