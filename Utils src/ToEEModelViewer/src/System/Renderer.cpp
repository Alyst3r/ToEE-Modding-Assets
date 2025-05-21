#include "Logger.hpp"
#include "Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <iostream>

static std::string loadFile(const char* path)
{
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void Renderer::initialize()
{
    // model shader
    const char* vertSrc = R"GLSL(
        #version 330 core

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aUV;
        layout (location = 2) in vec3 aNormal;

        out vec3 FragPos;
        out vec3 Normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )GLSL";

    const char* fragSrc = R"GLSL(
        #version 330 core

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

    // bone shader
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

    boneShaderProgram = glCreateProgram();
    glAttachShader(boneShaderProgram, boneVertexShader);
    glAttachShader(boneShaderProgram, boneFragmentShader);
    glLinkProgram(boneShaderProgram);

    glDeleteShader(boneVertexShader);
    glDeleteShader(boneFragmentShader);
}

void Renderer::shutdown()
{
    clearMesh();

    if (shaderProgram)
        glDeleteProgram(shaderProgram);

    if (gridShaderProgram)
        glDeleteProgram(gridShaderProgram);
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

void Renderer::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, bool uniformLighting)
{
    if (!mesh.vao)
        return;

    glUseProgram(shaderProgram);

    glm::mat4 model = mesh.modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, &lightDir[0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "uniformLight"), uniformLighting ? 1 : 0);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::renderGrid(const glm::mat4& view, const glm::mat4& projection) const
{
    glUseProgram(gridShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    const int gridSize = 20;
    const float spacing = 20.f * mesh.modelScale;

    std::vector<glm::vec3> lines;

    for (int i = -gridSize; i <= gridSize; ++i)
    {
        float pos = i * spacing;
        lines.push_back(glm::vec3(pos, 0.f, -gridSize * spacing));
        lines.push_back(glm::vec3(pos, 0.f, gridSize * spacing));

        lines.push_back(glm::vec3(-gridSize * spacing, 0.f, pos));
        lines.push_back(glm::vec3(gridSize * spacing, 0.f, pos));
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Renderer::renderBones(const glm::mat4& view, const glm::mat4& projection, float scaleFactor)
{
    glUseProgram(boneShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(boneShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(boneShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    for (const auto& bonePos : mesh.bonePositions)
    {
        float factor = 2.f * mesh.modelScale * scaleFactor;
        glm::vec3 minX = bonePos - (glm::vec3(1.f, 0.f, 0.f) * factor);
        glm::vec3 maxX = bonePos + (glm::vec3(1.f, 0.f, 0.f) * factor);
        glm::vec3 minY = bonePos - (glm::vec3(0.f, 1.f, 0.f) * factor);
        glm::vec3 maxY = bonePos + (glm::vec3(0.f, 1.f, 0.f) * factor);
        glm::vec3 minZ = bonePos - (glm::vec3(0.f, 0.f, 1.f) * factor);
        glm::vec3 maxZ = bonePos + (glm::vec3(0.f, 0.f, 1.f) * factor);
        glm::vec3 data[6] = { minX, maxX, minY, maxY, minZ, maxZ };
        glm::vec3 colors[3] = {
            glm::vec3(1.f, 0, 0),
            glm::vec3(0, 1.f, 0),
            glm::vec3(0, 0, 1.f)
        };

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
       
        for (int i = 0; i < 3; ++i)
        {
            glUniform3f(glGetUniformLocation(boneShaderProgram, "color"), colors[i].x, colors[i].y, colors[i].z);
            glDrawArrays(GL_LINES, i * 2, 2);
        }

        glBindVertexArray(0);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
}
