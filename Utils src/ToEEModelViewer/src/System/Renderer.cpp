#include "HelperShapes.hpp"
#include "Logger.hpp"
#include "Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

void Renderer::initialize()
{
    // model shader
    const char* vertSrc = R"GLSL(
        #version 330 core

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aUV;
        layout (location = 2) in vec3 aNormal;
        layout (location = 3) in uvec4 aBoneIDs;
        layout (location = 4) in vec4 aWeights;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform samplerBuffer boneMatrixTex;

        mat4 getBoneMatrix(int index) {
            int baseIndex = index * 4;
            return mat4(
                texelFetch(boneMatrixTex, baseIndex + 0),
                texelFetch(boneMatrixTex, baseIndex + 1),
                texelFetch(boneMatrixTex, baseIndex + 2),
                texelFetch(boneMatrixTex, baseIndex + 3)
            );
        }
        
        void main() {
            mat4 skinMatrix = 
                getBoneMatrix(int(aBoneIDs.x)) * aWeights.x +
                getBoneMatrix(int(aBoneIDs.y)) * aWeights.y +
                getBoneMatrix(int(aBoneIDs.z)) * aWeights.z +
                getBoneMatrix(int(aBoneIDs.w)) * aWeights.w;

            vec4 skinnedPosition = skinMatrix * vec4(aPos, 1.0);
            vec3 skinnedNormal = mat3(skinMatrix) * aNormal;

            FragPos = vec3(model * skinnedPosition);
            Normal = mat3(transpose(inverse(model))) * skinnedNormal;
            TexCoord = aUV;

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )GLSL";

    const char* fragSrc = R"GLSL(
        #version 330 core

        in vec2 TexCoord;
        in vec3 FragPos;
        in vec3 Normal;

        out vec4 FragColor;

        uniform vec3 lightDir;
        uniform vec3 baseColor = vec3(0.8, 0.8, 0.8);
        uniform bool uniformLight;

        void main() {
            float diff = uniformLight ? 1.0 : max(dot(normalize(Normal), -lightDir), 0.0);

            vec3 ambient = 0.1 * baseColor;
            vec3 result = ambient + baseColor * diff * vec3(1.0);
            FragColor = vec4(result, 1.0);
        }
    )GLSL";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSrc, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // grid shader
    const char* gridVertSrc = R"GLSL(
        #version 330 core
        layout (location = 0) in vec3 aPos;
    
        uniform mat4 view;
        uniform mat4 projection;
    
        void main() {
            gl_Position = projection * view * vec4(aPos, 1.0);
        }
    )GLSL";

    const char* gridFragSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
    
        void main() {
            FragColor = vec4(0.5, 0.5, 0.5, 1.0); // gray color
        }
    )GLSL";

    GLuint gridVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(gridVertexShader, 1, &gridVertSrc, nullptr);
    glCompileShader(gridVertexShader);

    GLuint gridFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(gridFragmentShader, 1, &gridFragSrc, nullptr);
    glCompileShader(gridFragmentShader);

    gridShaderProgram = glCreateProgram();
    glAttachShader(gridShaderProgram, gridVertexShader);
    glAttachShader(gridShaderProgram, gridFragmentShader);
    glLinkProgram(gridShaderProgram);

    glDeleteShader(gridVertexShader);
    glDeleteShader(gridFragmentShader);

    // bone axes shader
    const char* boneVertSrc = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 aPos;

        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * vec4(aPos, 1.0);
        }
    )GLSL";

    const char* boneFragSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;

        void main() {
            FragColor = vec4(color, 1.0);
        }
    )GLSL";

    GLuint boneVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(boneVertexShader, 1, &boneVertSrc, nullptr);
    glCompileShader(boneVertexShader);

    GLuint boneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(boneFragmentShader, 1, &boneFragSrc, nullptr);
    glCompileShader(boneFragmentShader);

    boneAxesShaderProgram = glCreateProgram();
    glAttachShader(boneAxesShaderProgram, boneVertexShader);
    glAttachShader(boneAxesShaderProgram, boneFragmentShader);
    glLinkProgram(boneAxesShaderProgram);

    glDeleteShader(boneVertexShader);
    glDeleteShader(boneFragmentShader);

    // bone shape shader
    const char* boneShapeVertSrc = R"GLSL(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 FragPos;
        out vec3 Normal;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )GLSL";

    const char* boneShapeFragSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        
        uniform vec3 lightDir;
        uniform vec3 objectColor = vec3(0.8, 0.5, 0.3);
        uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
        
        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDirection = normalize(-lightDir);

            vec3 ambient = 0.1 * objectColor;
            float diff = max(dot(norm, lightDirection), 0.0);
            vec3 diffuse = diff * lightColor * objectColor;
            vec3 finalColor = ambient + diffuse;

            FragColor = vec4(finalColor, 1.0);
        }
    )GLSL";

    GLuint boneShapeVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(boneShapeVertexShader, 1, &boneShapeVertSrc, nullptr);
    glCompileShader(boneShapeVertexShader);

    GLuint boneShapeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(boneShapeFragmentShader, 1, &boneShapeFragSrc, nullptr);
    glCompileShader(boneShapeFragmentShader);

    boneShapeShaderProgram = glCreateProgram();
    glAttachShader(boneShapeShaderProgram, boneShapeVertexShader);
    glAttachShader(boneShapeShaderProgram, boneShapeFragmentShader);
    glLinkProgram(boneShapeShaderProgram);

    glDeleteShader(boneShapeVertexShader);
    glDeleteShader(boneShapeFragmentShader);

    // other stuff
    // bone TBO
    glGenBuffers(1, &boneTBO);
    glGenTextures(1, &boneTBOTexture);

    // grid setup
    std::vector<glm::vec3> lines;
    const int gridSize = 50;
    const float spacing = 10.f;

    for (int i = -gridSize; i <= gridSize; ++i)
    {
        float pos = i * spacing;
        lines.push_back(glm::vec3(pos, 0.f, -gridSize * spacing));
        lines.push_back(glm::vec3(pos, 0.f, gridSize * spacing));

        lines.push_back(glm::vec3(-gridSize * spacing, 0.f, pos));
        lines.push_back(glm::vec3(gridSize * spacing, 0.f, pos));
    }

    gridLineCount = static_cast<int>(lines.size());

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void Renderer::shutdown()
{
    clearMesh();

    if (shaderProgram)
        glDeleteProgram(shaderProgram);

    if (gridShaderProgram)
        glDeleteProgram(gridShaderProgram);

    if (boneAxesShaderProgram)
        glDeleteProgram(boneAxesShaderProgram);

    if (boneShapeShaderProgram)
        glDeleteProgram(boneShapeShaderProgram);

    if (gridVAO)
        glDeleteVertexArrays(1, &gridVAO);

    if (gridVBO)
        glDeleteBuffers(1, &gridVBO);

    if (boneAxesVAO)
        glDeleteVertexArrays(1, &boneAxesVAO);

    if (boneAxesVBO)
        glDeleteBuffers(1, &boneAxesVBO);

    if (boneShapeVAO)

        glDeleteVertexArrays(1, &boneShapeVAO);
    if (boneShapeVBO)
        glDeleteBuffers(1, &boneShapeVBO);
}

void Renderer::uploadMesh(const SKM::MeshBuffer& inputMesh)
{
    clearMesh();
    mesh = inputMesh;
    mesh.upload();
}

void Renderer::clearMesh()
{
    mesh.destroy();
}

void Renderer::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, bool uniformLighting, bool showTPose)
{
    if (!mesh.modelVAO)
        return;

    glUseProgram(shaderProgram);

    glm::mat4 model = mesh.modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    // bones
    const std::vector<glm::mat4>& boneMats = showTPose ? mesh.tPoseSkinningMatrix : mesh.skinningMatrix;
    size_t boneCount = boneMats.size();

    glBindBuffer(GL_TEXTURE_BUFFER, boneTBO);
    glBufferData(GL_TEXTURE_BUFFER, boneCount * sizeof(glm::mat4), boneMats.data(), GL_DYNAMIC_DRAW);

    glBindTexture(GL_TEXTURE_BUFFER, boneTBOTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, boneTBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, boneTBOTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "boneMatrixTex"), 0);
    // bones end

    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, &lightDir[0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "uniformLight"), uniformLighting ? 1 : 0);

    glBindVertexArray(mesh.modelVAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::renderGrid(const glm::mat4& view, const glm::mat4& projection) const
{
    glUseProgram(gridShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridLineCount);
    glBindVertexArray(0);
}

void Renderer::renderBones(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, bool showAxes, bool showOctahedrons, const glm::vec3 lightDir, bool showTPose)
{
    if (!showAxes && !showOctahedrons)
    {
        renderBoneShapes(view, projection, scaleFactor, lightDir, showTPose);
        return;
    }

    if (showAxes)
        renderBoneAxes(view, projection, scaleFactor, showTPose);

    if (showOctahedrons)
        renderBoneShapes(view, projection, scaleFactor, lightDir, showTPose);
}

void Renderer::renderBoneAxes(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, bool showTPose)
{
    glUseProgram(boneAxesShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(boneAxesShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(boneAxesShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    for (const auto& boneMatrix : showTPose ? mesh.skmWorldMatrices : mesh.skaWorldMatrices)
    {
        float factor = 4.f * scaleFactor;
        glm::vec3 position = glm::vec3(boneMatrix[3]);
        glm::vec3 xAxis = glm::normalize(glm::vec3(boneMatrix[0])) * factor;
        glm::vec3 yAxis = glm::normalize(glm::vec3(boneMatrix[1])) * factor;
        glm::vec3 zAxis = glm::normalize(glm::vec3(boneMatrix[2])) * factor;

        glm::vec3 data[6] = {
            position - xAxis, position + xAxis,
            position - yAxis, position + yAxis,
            position - zAxis, position + zAxis
        };

        glm::vec3 colors[3] = {
            glm::vec3(1.f, 0, 0),
            glm::vec3(0, 1.f, 0),
            glm::vec3(0, 0, 1.f)
        };

        glGenVertexArrays(1, &boneAxesVAO);
        glGenBuffers(1, &boneAxesVBO);

        glBindVertexArray(boneAxesVAO);
        glBindBuffer(GL_ARRAY_BUFFER, boneAxesVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        for (int i = 0; i < 3; ++i)
        {
            glUniform3f(glGetUniformLocation(boneAxesShaderProgram, "color"), colors[i].x, colors[i].y, colors[i].z);
            glDrawArrays(GL_LINES, i * 2, 2);
        }

        glBindVertexArray(0);
        glDeleteBuffers(1, &boneAxesVBO);
        glDeleteVertexArrays(1, &boneAxesVAO);
    }
}

void Renderer::renderBoneShapes(const glm::mat4& view, const glm::mat4& projection, float scaleFactor, const glm::vec3 lightDir, bool showTPose)
{
    glUseProgram(boneShapeShaderProgram);

    glm::mat4 model = glm::mat4(1.f);
    glUniformMatrix4fv(glGetUniformLocation(boneShapeShaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(boneShapeShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(boneShapeShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(boneShapeShaderProgram, "lightDir"), 1, &lightDir[0]);

    for (const auto& boneMatrix : showTPose ? mesh.skmWorldMatrices : mesh.skaWorldMatrices)
    {
        Shape::Vertex transformedVertices[24];
        memcpy(transformedVertices, Shape::Bone::modelVertices, sizeof(Shape::Bone::modelVertices));

        for (uint8_t i = 0; i < 24; i++)
        {
            glm::vec4 worldPos = boneMatrix * glm::vec4(transformedVertices[i].position * scaleFactor * 7.5f, 1.0f);
            transformedVertices[i].position = glm::vec3(worldPos);

            glm::vec4 normal = glm::transpose(glm::inverse(boneMatrix)) * glm::vec4(transformedVertices[i].normal, 0.0f);
            transformedVertices[i].normal = glm::normalize(glm::vec3(normal));
        }

        glGenVertexArrays(1, &boneShapeVAO);
        glGenBuffers(1, &boneShapeVBO);

        glBindVertexArray(boneShapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, boneShapeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(transformedVertices), transformedVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Shape::Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Shape::Vertex), (void*)offsetof(Shape::Vertex, Shape::Vertex::normal));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, Shape::Bone::vertexCount);

        glBindVertexArray(0);
        glDeleteBuffers(1, &boneShapeVBO);
        glDeleteVertexArrays(1, &boneShapeVAO);
    }
}
