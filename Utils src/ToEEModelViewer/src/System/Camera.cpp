#include "Camera.hpp"

Camera::Camera()
{
    reset();
}

void Camera::reset()
{
    rotation = glm::quat(glm::vec3(0.f));
    target = glm::vec3(0.f);
    distance = 100.0f;
    pitchDeg = 0.f;
    yawDeg = 0.f;
}

void Camera::rotate(float yawDelta, float pitchDelta)
{
    glm::quat yawQuat = glm::angleAxis(yawDelta, glm::vec3(0.f, 1.f, 0.f));
    glm::vec3 right = glm::normalize(glm::cross(getForward(), glm::vec3(0.f, 1.f, 0.f)));
    glm::quat pitchQuat = glm::angleAxis(pitchDelta, right);

    rotation = glm::normalize(yawQuat * pitchQuat * rotation);

    syncEulerFromQuaternion();
}

void Camera::pan(const glm::vec2& delta, float viewportHeight)
{
    float panSpeed = distance * 0.00275f;

    glm::vec3 worldDelta = getRight() * -delta.x * panSpeed + getUp() * delta.y * panSpeed;
    target += worldDelta;
}

void Camera::zoom(float scrollDelta)
{
    distance -= scrollDelta;
    distance = glm::clamp(distance, 10.f, 10000.f);
}

glm::mat4 Camera::getViewMatrix() const
{
    glm::vec3 position = getPosition();
    return glm::lookAt(position, target, getUp());
}

glm::vec3 Camera::getPosition() const
{
    return target + rotation * glm::vec3(0.f, 0.f, distance);
}

glm::vec3 Camera::getBoneLightPosition() const
{
    return rotation * glm::vec3(0.f, 0.f, 1000.f);
}

glm::vec3 Camera::getForward() const
{
    return glm::normalize(target - getPosition());
}

glm::vec3 Camera::getRight() const
{
    return glm::normalize(glm::cross(getForward(), glm::vec3(0.f, 1.f, 0.f)));
}

glm::vec3 Camera::getUp() const
{
    return glm::normalize(glm::cross(getRight(), getForward()));
}

void Camera::setTarget(const glm::vec3& newTarget)
{
    target = newTarget;
}

void Camera::resetPanning()
{
    target = glm::vec3(0.f);
}

void Camera::resetZoom()
{
    distance = 100.0f;
}

void Camera::setDistance(float d)
{
    distance = glm::clamp(d, 0.1f, 10000.f);
}

void Camera::setEulerAngles(float yawDegInput, float pitchDegInput)
{
    yawDeg = glm::mod(yawDegInput + 180.f, 360.f) - 180.f;
    pitchDeg = glm::clamp(pitchDegInput, -89.999f, 89.999f);

    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchDeg), glm::vec3(1.f, 0.f, 0.f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yawDeg), glm::vec3(0.f, 1.f, 0.f));
    rotation = glm::normalize(yawQuat * pitchQuat);
}

void Camera::adjustEulerAngles(float yawDeltaDeg, float pitchDeltaDeg)
{
    setEulerAngles(yawDeg + yawDeltaDeg, pitchDeg + pitchDeltaDeg);
}

glm::vec2 Camera::getEulerAnglesDeg() const
{
    return { yawDeg, pitchDeg };
}

void Camera::syncEulerFromQuaternion()
{
    glm::vec3 angles = glm::degrees(glm::eulerAngles(rotation));
    yawDeg = glm::mod(angles.y + 180.f, 360.f) - 180.f;
    pitchDeg = glm::clamp(angles.x, -89.999f, 89.999f);
}
