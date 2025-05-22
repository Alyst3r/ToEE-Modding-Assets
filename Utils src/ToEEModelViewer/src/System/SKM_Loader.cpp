#include "Logger.hpp"
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
        skmFilename = path.substr(path.find_last_of("/\\") + 1);

        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            LOG_ERROR << "[SKM] Failed to open file: " << path << "\n";
            return false;
        }

        file.read(reinterpret_cast<char*>(&header), sizeof(Header));

        if (!file)
        {
            LOG_ERROR << "[SKM] Failed to read header.\n";
            return false;
        }

        file.seekg(header.boneDataOffset);
        bones.resize(header.boneCount);
        file.read(reinterpret_cast<char*>(bones.data()), bones.size() * sizeof(BoneData));

        file.seekg(header.materialDataOffset);
        std::vector<MaterialData> materialTemp(header.materialCount);
        materials.resize(header.materialCount);
        file.read(reinterpret_cast<char*>(materialTemp.data()), materialTemp.size() * sizeof(MaterialData));
        for (uint32_t i = 0; i < header.materialCount; i++)
        {
            std::string temp = materialTemp[i].materialFilePath;
            materials.emplace_back(temp);
        }

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
            GPUVertex out = {};
            out.position = glm::vec3(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            out.uv = glm::vec2(v.uvPosition.x, v.uvPosition.y);
            out.normal = glm::vec3(v.normals.x, v.normals.y, v.normals.z);
            mesh.vertices.emplace_back(out);

            glm::vec3 pos(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);
        }

        float min = glm::compMin(minPos);
        float max = glm::compMax(maxPos);
        float mult = glm::max(abs(min), max);

        float scale = 1.0f / mult;
        mesh.modelScale = scale;
        mesh.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

        mesh.indices.reserve(faces.size() * 3);
        for (const auto& f : faces)
        {
            mesh.indices.emplace_back(f.vertexIndex[0]);
            mesh.indices.emplace_back(f.vertexIndex[1]);
            mesh.indices.emplace_back(f.vertexIndex[2]);
        }

        mesh.bonePositions.reserve(bones.size());
        for (const auto& b : bones)
        {
            glm::mat4 tempMatrix = toMat(b.worldInverse);
            tempMatrix = glm::inverse(tempMatrix);

#ifndef NDEBUG
            LOG_DEBUG << "===============SKM to render data================";
            LOG_DEBUG << "Current bone: " << b.boneName;
            b.parentBone > -1 ? LOG_DEBUG << "Parent: " << bones[b.parentBone].boneName : LOG_DEBUG << "Parent: " << "None";
            LOG_DEBUG << "[";
            LOG_DEBUG << tempMatrix[0][0] << " " << tempMatrix[0][1] << " " << tempMatrix[0][2] << " " << tempMatrix[0][3];
            LOG_DEBUG << tempMatrix[1][0] << " " << tempMatrix[1][1] << " " << tempMatrix[1][2] << " " << tempMatrix[1][3];
            LOG_DEBUG << tempMatrix[2][0] << " " << tempMatrix[2][1] << " " << tempMatrix[2][2] << " " << tempMatrix[2][3];
            LOG_DEBUG << tempMatrix[3][0] << " " << tempMatrix[3][1] << " " << tempMatrix[3][2] << " " << tempMatrix[3][3];
            LOG_DEBUG << "]";
#endif

            glm::vec3 temp = glm::vec3(tempMatrix[3][0] * scale, tempMatrix[3][1] * scale, tempMatrix[3][2] * scale);
            mesh.bonePositions.emplace_back(temp);
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

    glm::mat4 toMat(const SKM::Matrix3x4& matrix)
    {
        return glm::mat4(
            matrix.rows[0].x, matrix.rows[1].x, matrix.rows[2].x, 0.f,
            matrix.rows[0].y, matrix.rows[1].y, matrix.rows[2].y, 0.f,
            matrix.rows[0].z, matrix.rows[1].z, matrix.rows[2].z, 0.f,
            matrix.rows[0].w, matrix.rows[1].w, matrix.rows[2].w, 1.f
        );
    }
}
