#include "Camera.h"

void Camera::UpdateCameraVectors()
{
    //鼠标移动
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->cameraZAxis = front;

    //cout << this->cameraZAxis.x << ", " << this->cameraZAxis.y << ", " << this->cameraZAxis.z << endl;
    //摄像机坐标系
    this->cameraZAxis = glm::normalize(this->cameraZAxis);
    this->cameraXAxis = glm::normalize(glm::cross(this->cameraZAxis, this->cameraYAxis));
    this->cameraYAxis = glm::normalize(glm::cross(this->cameraXAxis, this->cameraZAxis));
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(this->position, this->position + this->cameraZAxis, this->cameraYAxis);      //观察矩阵
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = this->movementSpeed * deltaTime;
    if (direction == FORWARD)
        this->position += this->cameraZAxis * velocity;

    if (direction == BACKWARD)
        this->position -= this->cameraZAxis * velocity;

    if (direction == LEFT)
        this->position -= this->cameraXAxis * velocity;

    if (direction == RIGHT)
        this->position += this->cameraXAxis * velocity;

    if (direction == UPWARD)
        this->position += this->cameraYAxis * velocity;

    if (direction == DOWNWARD)
        this->position -= this->cameraYAxis * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    //防止出现倒立看大地的情况
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
