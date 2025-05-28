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
        if (file.gcount() != 18)
            return false;

        const uint8_t imageType = header[2];
        const uint8_t bitsPerPixel = header[16];
        const uint8_t bytesPerPixel = bitsPerPixel / 8;

        if (imageType != 2 || (bitsPerPixel != 24 && bitsPerPixel != 32))
            return false;

        outImage.width = header[12] | (header[13] << 8);
        outImage.height = header[14] | (header[15] << 8);
        size_t pixelCount = static_cast<size_t>(outImage.width) * outImage.height;

        std::vector<uint8_t> rawData(pixelCount * bytesPerPixel);
        file.read(reinterpret_cast<char*>(rawData.data()), rawData.size());
        if (!file)
            return false;

        outImage.pixels.resize(pixelCount * 4);

        for (size_t i = 0; i < pixelCount; ++i)
        {
            uint8_t b = rawData[i * bytesPerPixel + 0];
            uint8_t g = rawData[i * bytesPerPixel + 1];
            uint8_t r = rawData[i * bytesPerPixel + 2];
            uint8_t a = (bytesPerPixel == 4) ? rawData[i * 4 + 3] : 255;

            outImage.pixels[i * 4 + 0] = r;
            outImage.pixels[i * 4 + 1] = g;
            outImage.pixels[i * 4 + 2] = b;
            outImage.pixels[i * 4 + 3] = a;
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
