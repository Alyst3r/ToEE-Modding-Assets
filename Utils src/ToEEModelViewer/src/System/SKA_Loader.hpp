#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SKA
{
    struct Vec3f
    {
        float x;
        float y;
        float z;
    };

    struct Vec4f
    {
        float x;
        float y;
        float z;
        float w;
    };

#pragma pack(push, 1)
    struct Header
    {
        uint32_t boneCount;
        uint32_t boneDataOffset;
        uint32_t deprecated[2];
        uint32_t animCount;
        uint32_t animDataOffset;
    };

    struct BoneData
    {
        uint16_t flags;
        uint16_t parentBone;
        char boneName[48];
        Vec3f scale;
        uint32_t padding0;
        Vec4f rotQuaternions;
        Vec3f position;
        uint32_t padding1;
    };
#pragma pack(pop)
    
    struct BoneTransform
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 position;
    };

    struct SKAFile
    {
        Header header = { };
        std::vector<BoneData> boneData;
        std::vector<BoneTransform> boneTransforms;

        bool loadFromFile(const std::string& path);
        void computeTransforms();
        void SKAFile::clear();
    };
}
