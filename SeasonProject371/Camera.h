#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

static float YAW = -90.0f;
static float PITCH = 0.0f;
static float SPEED = 96.0f;
static float SENSITIVITY = 0.04f;
static float ZOOM =  45.0f;

class Camera {
    public:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 worldUp;
        float yaw;
        float pitch;
        float movementSpeed;
        float mouseSensitivity;
        float zoom;
        float lastFrame = 0.0f;

        Camera(glm::vec3);

        glm::mat4 GetViewMatrix();
        void processKeyboard(CameraMovement direction, float time);
        void processMouseMovement(float offsetX, float offsetY);
        void processMouseScroll(float offsetY);

        void updateCameraVectors();
};
