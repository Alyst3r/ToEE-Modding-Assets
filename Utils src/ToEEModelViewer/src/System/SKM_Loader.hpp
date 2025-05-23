#pragma once

#include "MDF_Loader.hpp"
#include "SKA_Loader.hpp"
#include "TGA_Loader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace SKM
{
    struct Vec2f
    {
        float x;
        float y;
    };
    struct Vec4f
    {
        float x;
        float y;
        float z;
        float w;
    };

    struct Matrix3x4
    {
        Vec4f rows[3];
    };

#pragma pack(push, 1)
    struct Header
    {
        uint32_t boneCount;
        uint32_t boneDataOffset;
        uint32_t materialCount;
        uint32_t materialDataOffset;
        uint32_t vertexCount;
        uint32_t vertexDataOffset;
        uint32_t faceCount;
        uint32_t faceDataOffset;
    };

    struct BoneData
    {
        uint16_t flags;
        int16_t parentBone;
        char boneName[48];
        Matrix3x4 worldInverse;
    };

    struct MaterialData
    {
        char materialFilePath[128];
    };

    struct VertexData
    {
        Vec4f vertexPosition;
        Vec4f normals;
        Vec2f uvPosition;
        uint16_t unknown;
        uint16_t vertexWeightsCount;
        uint16_t boneID[6];
        float boneWeight[6];
    };

    struct FaceData
    {
        uint16_t materialIndex;
        uint16_t vertexIndex[3];
    };
#pragma pack(pop)

    struct GPUVertex
    {
        glm::vec3 position = glm::vec3(0.f);
        glm::vec2 uv = glm::vec2(0.f);
        glm::vec3 normal = glm::vec3(0.f);

        glm::uvec4 boneIDs = glm::uvec4(0);
        glm::vec4 boneWeights = glm::vec4(0.0f);
    };

    struct MeshBuffer
    {
        std::vector<GPUVertex> vertices;
        std::vector<uint32_t> indices;

        std::vector<glm::mat4> skaWorldMatrices;
        std::vector<glm::mat4> skmWorldMatrices;
        std::vector<glm::mat4> skinningMatrix;
        std::vector<glm::mat4> tPoseSkinningMatrix;

        std::vector<MDF::MDFFile> materialData;
        std::unordered_map<std::string, TGA::TGAImage> textureCache;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec3 modelCenter = glm::vec3(0.0f);

        GLuint modelVAO = 0;
        GLuint modelVBO = 0;
        GLuint modelEBO = 0;

        void upload();
        void destroy();

        void loadTextures();
    };

    struct SKMFile
    {
        Header header = { 0 };
        std::vector<BoneData> bones;
        std::vector<std::string> materials;
        std::vector<VertexData> vertices;
        std::vector<FaceData> faces;

        std::vector<glm::mat4> skaWorldMatrices;
        std::vector<glm::mat4> skmInverseWorldMatrices;
        std::vector<glm::mat4> skmWorldMatrices;

        std::string rootPath;
        std::string skmFilename;

        bool loaded = false;

        void SKMFile::clear();

        bool loadAnimation(const std::string& path);
        bool loadFromFile(const std::string& path);
        void loadMaterials();

        MeshBuffer toMesh();
        SKA::SKAFile animation;
        std::vector<MDF::MDFFile> materialData;
    };

    glm::mat4 toMat(const SKM::Matrix3x4& matrix);
}
