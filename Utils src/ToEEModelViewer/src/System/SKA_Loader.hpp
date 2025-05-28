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
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
    };

    struct Vec4f
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        float w = 0.f;
    };

#pragma pack(push, 1)
    struct Header
    {
        uint32_t boneCount = 0;
        uint32_t boneDataOffset = 0;
        uint32_t deprecated[2] = { 0 };
        uint32_t animCount = 0;
        uint32_t animDataOffset = 0;
    };

    struct BoneData
    {
        int16_t flags = 0;
        int16_t parentBone = -1;
        char boneName[48];
        Vec3f scale;
        uint32_t padding0 = 0;
        Vec4f rotQuaternions;
        Vec3f position;
        uint32_t padding1 = 0;
    };

    struct AnimationStreamHeader
    {
        int16_t frameCount = 0;
        int16_t variationId = -1;
        float frameRate = 30.f;
        float drawingRate = 0.f;
        uint32_t dataOffset = 0;
    };

    struct AnimationHeader
    {
        char name[64];
        uint8_t driveType = 0;
        uint8_t loopable = 0;
        int16_t eventCount = 0;
        uint32_t eventOffset = 0;
        int16_t streamCount = 0;
        uint16_t padding = 0;
        AnimationStreamHeader streamHeaderData[10];
    };

    struct AnimationEvent
    {
        int16_t frameId = 0;
        char eventType[48];
        char action[128];
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
        std::vector<AnimationHeader> animHeaderData;
        std::vector<AnimationEvent> animEventData;

        std::vector<BoneTransform> boneTransforms;

        bool loadFromFile(const std::string& path);
        void computeTransforms();
        int16_t computeAnimEventCount();
        void SKAFile::clear();
    };
}
