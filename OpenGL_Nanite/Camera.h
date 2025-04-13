#pragma once
#include "OpenGLwin.h"

enum CameraMovement {
	FORWARD,		// ��ǰ
	BACKWARD,		// ���
	LEFT,			// ����
	RIGHT,			// ����
	UPWARD,			// ����
	DOWNWARD		// ����
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
private:
	// ��Ӱ��������
	glm::vec3 position;	//λ��
	glm::vec3 front;	//�������Z��
	glm::vec3 up;	//�������Y��
	glm::vec3 right;	//�������X��
	glm::vec3 worldUp;	//�����µ�Y��

	GLfloat movementSpeed;			// ��ͷ�ƶ��ٶ�

	float yaw;		//������
	float pitch;		//ƫ�ƽ�
	float mouseSensitivity;	//���������
	float zoom;		//ͶӰ���

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

	//��õ�ǰ���������Ұ
	GLfloat GetZoom();

	//���̿��Ƶ��ӽǱ仯
	void ProcessKeyboard(CameraMovement direction, float deltaTime);

	//����ƶ����ӽǱ仯
	void ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch = true);

	//����������
	void ProcessMouseScroll(float yOffset);
};
