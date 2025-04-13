#include "Camera.h"

void Camera::UpdateCameraVectors()
{
    //����ƶ�
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = front;

    //cout << this->cameraZAxis.x << ", " << this->cameraZAxis.y << ", " << this->cameraZAxis.z << endl;
    //���������ϵ
    this->front = glm::normalize(this->front);
    this->right = glm::normalize(glm::cross(this->front, this->worldUp));
    this->up = glm::normalize(glm::cross(this->right, this->front));
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(this->position, this->position + this->front, this->worldUp);      //�۲����
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = static_cast<float>(this->movementSpeed * deltaTime);
    if (direction == FORWARD)
        this->position += this->front * velocity;

    if (direction == BACKWARD)
        this->position -= this->front * velocity;

    if (direction == LEFT)
        this->position -= this->right * velocity;

    if (direction == RIGHT)
        this->position += this->right * velocity;

    if (direction == UPWARD)
        this->position += this->up * velocity;

    if (direction == DOWNWARD)
        this->position -= this->up * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    //��ֹ���ֵ�������ص����
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
    this->UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yOffset)
{
    zoom -= yOffset;
    if (zoom < 1.0f) {
        zoom = 1.0f;
    }
    if (zoom > 45.0f) {
        zoom = 45.0f;
    }
}

glm::vec3 Camera::GetPosition()
{
    return this->position;
}

GLfloat Camera::GetZoom()
{
    return this->zoom;
}
