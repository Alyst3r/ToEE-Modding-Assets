#include "TGA_Loader.hpp"

#include <fstream>

namespace TGA
{
    bool loadTGA(const std::string& filepath, TGAImage& outImage)
    {
        std::ifstream file(filepath, std::ios::binary);

        if (!file)
            return false;

        uint8_t header[18];

        file.read(reinterpret_cast<char*>(header), 18);
        if (file.gcount() != 18 || header[2] != 2 || header[16] != 32)
        {
            return false;
        }

        outImage.width = header[12] | (header[13] << 8);
        outImage.height = header[14] | (header[15] << 8);

        size_t pixelCount = static_cast<size_t>(outImage.width) * outImage.height;
        outImage.pixels.resize(pixelCount * 4);

        file.read(reinterpret_cast<char*>(outImage.pixels.data()), pixelCount * 4);
        if (!file) return false;

        for (size_t i = 0; i < pixelCount; ++i)
        {
            std::swap(outImage.pixels[i * 4 + 0], outImage.pixels[i * 4 + 2]);
        }

        return true;
    }

    TGAImage& getOrLoadTexture(const std::string& fullPath, std::unordered_map<std::string, TGAImage>& textureCache)
    {
        auto it = textureCache.find(fullPath);

        if (it != textureCache.end())
            return it->second;

        TGAImage tex;
        if (loadTGA(fullPath, tex))
        {
            textureCache[fullPath] = std::move(tex);
            return textureCache[fullPath];
        }

        throw std::runtime_error("Failed to load TGA: " + fullPath);
    }
}
