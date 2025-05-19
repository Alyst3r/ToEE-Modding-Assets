#include "SKM_Loader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>

#include <fstream>
#include <iostream>

namespace SKM
{
    bool SKMFile::loadFromFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            std::cerr << "[SKM] Failed to open file: " << path << "\n";
            return false;
        }

        file.read(reinterpret_cast<char*>(&header), sizeof(Header));

        if (!file)
        {
            std::cerr << "[SKM] Failed to read header.\n";
            return false;
        }

        file.seekg(header.boneDataOffset);
        bones.resize(header.boneCount);
        file.read(reinterpret_cast<char*>(bones.data()), bones.size() * sizeof(BoneData));

        file.seekg(header.materialDataOffset);
        materials.resize(header.materialCount);
        file.read(reinterpret_cast<char*>(materials.data()), materials.size() * sizeof(MaterialData));

        file.seekg(header.vertexDataOffset);
        vertices.resize(header.vertexCount);
        file.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(VertexData));

        file.seekg(header.faceDataOffset);
        faces.resize(header.faceCount);
        file.read(reinterpret_cast<char*>(faces.data()), faces.size() * sizeof(FaceData));

        return true;
    }

    MeshBuffer SKMFile::toMesh()
    {
        MeshBuffer mesh;
        glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);

        mesh.vertices.reserve(vertices.size());
        for (const auto& v : vertices)
        {
            GPUVertex out;
            out.position = glm::vec3(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            out.uv = glm::vec2(v.uvPosition.x, v.uvPosition.y);
            out.normal = glm::vec3(v.normals.x, v.normals.y, v.normals.z);
            mesh.vertices.push_back(out);

            glm::vec3 pos(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);
        }

        glm::vec3 size = maxPos - minPos;

        if (minPos.y > 0.0f)
            minPos.y = 0.0f;

        glm::vec3 center = (minPos + maxPos) * 0.5f;
        center.y = minPos.y;
        mesh.modelCenter = center;

        float scale = 1.0f / glm::compMax(maxPos);
        mesh.modelScale = scale;

        glm::mat4 translate = glm::translate(glm::mat4(1.0f), -center);
        glm::mat4 uniformScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        mesh.modelMatrix = uniformScale * translate;
        
        mesh.indices.reserve(faces.size() * 3);
        for (const auto& f : faces)
        {
            mesh.indices.push_back(f.vertexIndex[0]);
            mesh.indices.push_back(f.vertexIndex[1]);
            mesh.indices.push_back(f.vertexIndex[2]);
        }

        return mesh;
    }

    void MeshBuffer::upload()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GPUVertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)offsetof(GPUVertex, uv));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)offsetof(GPUVertex, normal));

        glBindVertexArray(0);
    }

    void MeshBuffer::destroy()
    {
        if (vao)
            glDeleteVertexArrays(1, &vao);

        if (vbo)
            glDeleteBuffers(1, &vbo);

        if (ebo)
            glDeleteBuffers(1, &ebo);

        vao = vbo = ebo = 0;
    }
}
