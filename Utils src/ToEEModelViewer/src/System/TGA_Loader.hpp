#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace TGA
{
    struct TGAImage
    {
        uint16_t width = 0;
        uint16_t height = 0;
        std::vector<uint8_t> pixels;
    };

    bool loadTGA(const std::string& filepath, TGAImage& outImage);

    TGAImage& getOrLoadTexture(const std::string& fullPath, std::unordered_map<std::string, TGAImage>& textureCache);
}
