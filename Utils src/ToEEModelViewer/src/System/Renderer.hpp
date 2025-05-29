#pragma once

#include <glm/glm.hpp>

#include "SKM_Loader.hpp"

class Renderer
{
public:
    void initialize();
    void shutdown();

    void uploadMesh(const SKM::MeshBuffer& mesh);
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, bool uniformLighting, bool showTPose);
    void renderGrid(const glm::mat4& view, const glm::mat4& projection) const;
    void renderBones(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, bool showAxes, bool showOctahedrons, const glm::vec3 lightDir, bool showTPose);
    void clearMesh();

    glm::vec3 getModelCenter() const { return mesh.modelCenter; };

private:
    GLuint shaderProgram = 0;
    GLuint gridShaderProgram = 0;
    GLuint boneAxesShaderProgram = 0;
    GLuint boneShapeShaderProgram = 0;

    uint32_t gridLineCount = 0;
    GLuint gridVAO = 0;
    GLuint gridVBO = 0;

    GLuint boneAxesVAO = 0;
    GLuint boneAxesVBO = 0;
    GLuint boneShapeVAO = 0;
    GLuint boneShapeVBO = 0;

    GLuint boneTBO = 0;
    GLuint boneTBOTexture = 0;

    SKM::MeshBuffer mesh;

    bool debugMaterials = false;

    void renderBoneAxes(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, bool showTPose);
    void renderBoneShapes(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, const glm::vec3 lightDir, bool showTPose);

    std::vector<glm::vec4> generateDebugColors(size_t count);
};
