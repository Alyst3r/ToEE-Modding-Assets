#pragma once

#include <glm/glm.hpp>

#include "SKM_Loader.hpp"

class Renderer
{
public:
    void initialize();
    void shutdown();

    void uploadMesh(const SKM::MeshBuffer& mesh);
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, bool uniformLighting);
    void renderGrid(const glm::mat4& view, const glm::mat4& projection) const;
    void renderBones(const glm::mat4& view, const glm::mat4& projection, float scaleFactor);
    void clearMesh();

private:
    GLuint shaderProgram = 0;
    GLuint gridShaderProgram = 0;
    GLuint boneShaderProgram = 0;

    SKM::MeshBuffer mesh;
};
