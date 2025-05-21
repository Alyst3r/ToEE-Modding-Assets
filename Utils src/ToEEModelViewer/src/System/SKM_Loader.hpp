#pragma once

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
    struct Header {
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
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    struct MeshBuffer
    {
        std::vector<GPUVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> bonePositions;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec3 modelCenter = glm::vec3(0.0f);

        float modelScale = 1.f;

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        void upload();
        void destroy();
    };

    struct SKMFile
    {
        Header header = { 0 };
        std::vector<BoneData> bones;
        std::vector<std::string> materials;
        std::vector<VertexData> vertices;
        std::vector<FaceData> faces;

        bool loadFromFile(const std::string& path);

        std::string skmFilename;
        MeshBuffer toMesh();
    };

    glm::mat4 toMat(const SKM::Matrix3x4& matrix);
}
