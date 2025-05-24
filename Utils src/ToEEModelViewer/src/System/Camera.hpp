#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera();

    void reset();
    void rotate(float yawDelta, float pitchDelta);
    void pan(const glm::vec2& delta, float viewportHeight);
    void zoom(float scrollDelta);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getBoneLightPosition() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    void setTarget(const glm::vec3& target);
    void resetPanning();
    void resetZoom();
    void setDistance(float d);

    void setEulerAngles(float yawDeg, float pitchDeg);
    void adjustEulerAngles(float yawDeltaDeg, float pitchDeltaDeg);
    glm::vec2 getEulerAnglesDeg() const;

    const glm::vec3& getTarget() const { return target; }
    float getDistance() const { return distance; }
    glm::quat getRotation() const { return rotation; }

private:
    glm::quat rotation;
    glm::vec3 target;
    float distance;
    float yawDeg = 0.f;
    float pitchDeg = 0.f;

    void syncEulerFromQuaternion();
};
