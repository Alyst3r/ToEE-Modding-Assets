#include "Logger.hpp"
#include "SKM_Loader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fstream>
#include <iostream>

namespace SKM
{
    bool SKMFile::loadFromFile(const std::string& path)
    {
        skmFilename = path.substr(path.find_last_of("/\\") + 1);

        if (!rootPath.length())
        {
            size_t pos = path.find("/art/");

            if (pos != std::string::npos)
                rootPath = path.substr(0, pos + 1);
            else
                LOG_ERROR << "\"" << path << "\" is not a valid Temple of Elemental Evil data directory.";
        }

        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            LOG_ERROR << "[SKM] Failed to open file: " << path;
            return false;
        }

        file.read(reinterpret_cast<char*>(&header), sizeof(Header));
        if (!file)
        {
            LOG_ERROR << "[SKM] Failed to read header.";
            return false;
        }

        file.seekg(header.boneDataOffset);
        bones.resize(header.boneCount);
        file.read(reinterpret_cast<char*>(bones.data()), bones.size() * sizeof(BoneData));

        skmInverseWorldMatrices.resize(header.boneCount);
        skmWorldMatrices.resize(header.boneCount);
        for (uint32_t i = 0; i < header.boneCount; i++)
        {
            glm::mat4 temp = toMat(bones[i].worldInverse);
            skmInverseWorldMatrices[i] = temp;
            skmWorldMatrices[i] = glm::inverse(temp);
        }

        file.seekg(header.materialDataOffset);
        std::vector<MaterialData> materialTemp(header.materialCount);
        materials.resize(header.materialCount);
        file.read(reinterpret_cast<char*>(materialTemp.data()), materialTemp.size() * sizeof(MaterialData));
        for (uint32_t i = 0; i < header.materialCount; i++)
        {
            std::string temp = materialTemp[i].materialFilePath;
            materials[i] = materialTemp[i].materialFilePath;
        }

        file.seekg(header.vertexDataOffset);
        vertices.resize(header.vertexCount);
        file.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(VertexData));

        file.seekg(header.faceDataOffset);
        faces.resize(header.faceCount);
        file.read(reinterpret_cast<char*>(faces.data()), faces.size() * sizeof(FaceData));

        // ska part
        std::string skaFilepath = path.substr(0, path.size() - 1) + "A";
        
        if (loadAnimation(skaFilepath))
        {
            skaWorldMatrices.resize(bones.size());
            for (size_t i = 0; i < bones.size(); i++)
            {
                const auto& t = animation.boneTransforms[i];
                glm::mat4 T = glm::translate(glm::mat4(1.0f), t.position);
                glm::mat4 R = glm::toMat4(t.rotation);
                glm::mat4 S = glm::scale(glm::mat4(1.0f), t.scale);
                glm::mat4 localMatrix = T * R * S;
                int parent = bones[i].parentBone;

                if (parent >= 0)
                {
                    skaWorldMatrices[i] = skaWorldMatrices[parent] * localMatrix;
                }
                else
                {
                    skaWorldMatrices[i] = localMatrix;
                }
            }
        }

        loadMaterials();

        loaded = true;

        return true;
    }

    void SKMFile::clear()
    {
        header = { 0 };
        bones.resize(0);
        materials.resize(0);
        vertices.resize(0);
        faces.resize(0);
        skaWorldMatrices.resize(0);
        skmInverseWorldMatrices.resize(0);
        skmWorldMatrices.resize(0);
        loaded = false;
        animation.clear();
        materialData.resize(0);
    }

    MeshBuffer SKMFile::toMesh()
    {
        uint32_t vertexID = 0;
        MeshBuffer mesh;
        glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);

        mesh.vertices.reserve(vertices.size());
        for (const auto& v : vertices)
        {
            GPUVertex out = {};
            out.position = glm::vec3(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            out.uv = glm::vec2(v.uvPosition.x, v.uvPosition.y);
            out.normal = glm::vec3(v.normals.x, v.normals.y, v.normals.z);

            if (v.vertexWeightsCount > 0)
            {
                if (v.vertexWeightsCount <= 4)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        out.boneIDs[i] = v.boneID[i];
                        out.boneWeights[i] = v.boneWeight[i];
                    }
                }
                else
                {
                    LOG_WARN << skmFilename << ": more than 4 vertex weights, vertex ID: " << vertexID << ", number of weights: " << v.vertexWeightsCount;

                    for (int i = 0; i < v.vertexWeightsCount; i++)
                    {
                        LOG_WARN << bones[v.boneID[i]].boneName << ": bone ID (" << v.boneID[i] << "), bone weight(" << v.boneWeight[i] <<")";
                    }

                    std::vector<std::pair<float, uint16_t>> weights;
                    for (int i = 0; i < v.vertexWeightsCount; i++)
                    {
                        weights.emplace_back(v.boneWeight[i], v.boneID[i]);
                    }
                    
                    std::sort(weights.begin(), weights.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

                    float total = 0.0f;
                    for (int i = 0; i < 4; ++i)
                    {
                        out.boneWeights[i] = weights[i].first;
                        out.boneIDs[i] = weights[i].second;
                        total += weights[i].first;
                    }

                    if (total > 0.0f)
                    {
                        for (int i = 0; i < 4; ++i)
                        {
                            out.boneWeights[i] /= total;
                        }
                    }
                    else
                    {
                        LOG_WARN << "Sheeiiit. Sum of selected weights is 0. This should never happen.";
                    }
                }
            }

            mesh.vertices.emplace_back(out);

            glm::vec3 pos(v.vertexPosition.x, v.vertexPosition.y, v.vertexPosition.z);
            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);

            vertexID++;
        }

        float min = glm::compMin(minPos);
        float max = glm::compMax(maxPos);
        mesh.modelCenter = (minPos + maxPos) * .5f;

        mesh.indices.reserve(faces.size() * 3);
        for (const auto& f : faces)
        {
            mesh.indices.emplace_back(f.vertexIndex[0]);
            mesh.indices.emplace_back(f.vertexIndex[1]);
            mesh.indices.emplace_back(f.vertexIndex[2]);
        }

        mesh.skinningMatrix.resize(skaWorldMatrices.size());
        for (size_t i = 0; i < skaWorldMatrices.size(); i++)
            mesh.skinningMatrix[i] = skaWorldMatrices[i] * skmInverseWorldMatrices[i];

        mesh.tPoseSkinningMatrix.resize(skaWorldMatrices.size());
        for (size_t i = 0; i < skaWorldMatrices.size(); i++)
            mesh.tPoseSkinningMatrix[i] = glm::mat4(1.f);

        mesh.skaWorldMatrices.resize(skaWorldMatrices.size());
        mesh.skaWorldMatrices = skaWorldMatrices;

        mesh.skmWorldMatrices.resize(skmWorldMatrices.size());
        mesh.skmWorldMatrices = skmWorldMatrices;

        mesh.materialData.resize(materialData.size());
        mesh.materialData = materialData;

        mesh.loadTextures();

        return mesh;
    }

    void MeshBuffer::upload()
    {
        glGenVertexArrays(1, &modelVAO);
        glGenBuffers(1, &modelVBO);
        glGenBuffers(1, &modelEBO);

        glBindVertexArray(modelVAO);

        glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GPUVertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)offsetof(GPUVertex, uv));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)offsetof(GPUVertex, normal));

        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(GPUVertex), (void*)offsetof(GPUVertex, boneIDs));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), (void*)offsetof(GPUVertex, boneWeights));

        glBindVertexArray(0);
    }

    void MeshBuffer::destroy()
    {
        if (modelVAO)
            glDeleteVertexArrays(1, &modelVAO);

        if (modelVBO)
            glDeleteBuffers(1, &modelVBO);

        if (modelEBO)
            glDeleteBuffers(1, &modelEBO);

        modelVAO = 0;
        modelVBO = 0;
        modelEBO = 0;

        vertices.resize(0);
        indices.resize(0);

        skaWorldMatrices.resize(0);
        skmWorldMatrices.resize(0);
        skinningMatrix.resize(0);
        tPoseSkinningMatrix.resize(0);

        materialData.resize(0);
        textureCache.clear();

        modelMatrix = glm::mat4(1.0f);
        modelCenter = glm::vec3(0.0f);
    }

    void MeshBuffer::loadTextures()
    {
        for (size_t i = 0; i < materialData.size(); i++)
        {
            for (int j = 0; j < materialData[i].textureCount; j++)
            {
                TGA::getOrLoadTexture(materialData[i].texturePath[j], textureCache);
            }

            if (materialData[i].glossMap.length())
                TGA::getOrLoadTexture(materialData[i].glossMap, textureCache);
        }
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

    bool SKMFile::loadAnimation(const std::string& path)
    {
        return animation.loadFromFile(path);
    }

    void SKMFile::loadMaterials()
    {
        materialData.resize(header.materialCount);

        for (size_t i = 0; i < materialData.size(); i++)
        {
            if (!materialData[i].parseMDFFile(rootPath, materials[i]))
            {
                LOG_ERROR << "Failed to parse material file: " << materials[i];
            }

#ifndef NDEBUG
            materialData[i].debugPrint();
#endif
        }
    }

    bool SKMFile::populateAnimNames(std::vector<std::string>& animList)
    {
        if (loaded)
        {
            animList.resize(0);
            animList.reserve(animation.header.animCount);

            for (const auto& it : animation.animHeaderData)
                animList.emplace_back(it.name);
        }

        return true;
    }
}
