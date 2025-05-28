#include "Logger.hpp"
#include "SKA_Loader.hpp"

#include <fstream>
#include <iostream>

namespace SKA
{
    bool SKAFile::loadFromFile(const std::string& path)
    {
        clear();

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            LOG_ERROR << "[SKA] Failed to open file: " << path;
            return false;
        }

        file.read(reinterpret_cast<char*>(&header), sizeof(Header));
        if (!file)
        {
            LOG_ERROR << "[SKM] Failed to read header.";
            return false;
        }

        file.seekg(header.boneDataOffset);
        boneData.resize(header.boneCount);
        file.read(reinterpret_cast<char*>(boneData.data()), boneData.size() * sizeof(BoneData));

        file.seekg(header.animDataOffset);
        animHeaderData.resize(header.animCount);
        file.read(reinterpret_cast<char*>(animHeaderData.data()), animHeaderData.size() * sizeof(AnimationHeader));

        computeTransforms();

        return true;
    }

    void SKAFile::computeTransforms()
    {
        boneTransforms.reserve(boneData.size());

        for (const auto& b : boneData)
        {
            BoneTransform transform = { };
            transform.scale = glm::vec3(b.scale.x, b.scale.y, b.scale.z);
            transform.rotation = glm::quat(b.rotQuaternions.w, b.rotQuaternions.x, b.rotQuaternions.y, b.rotQuaternions.z);
            transform.position = glm::vec3(b.position.x, b.position.y, b.position.z);
            boneTransforms.push_back(transform);
        }
    }

    void SKAFile::clear()
    {
        header = { };
        boneData.resize(0);
        boneTransforms.resize(0);
    }
}
