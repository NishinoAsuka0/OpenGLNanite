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

const GLfloat SPEED = 6.0f;

class Camera {
private:
	// ��Ӱ��������
	glm::vec3 position;				// �����ǰλ�� 
	glm::vec3 cameraZAxis;		// ��Ӱ���� Z ������
	glm::vec3 cameraXAxis;		// ��Ӱ���� X ������
	glm::vec3 cameraYAxis;		// ��Ӱ���� Y ������
	GLfloat movementSpeed;			// ��ͷ�ƶ��ٶ�

	float yaw;		//������
	float pitch;		//ƫ�ƽ�
	float mouseSensitivity;	//���������
	float zoom;		//ͶӰ���

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
		this->UpdateCameraVectors();		// ʵʱ����
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