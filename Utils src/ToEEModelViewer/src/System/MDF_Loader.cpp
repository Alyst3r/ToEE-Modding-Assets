#include "Logger.hpp"
#include "MDF_Loader.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace MDF
{
    static std::string toLower(const std::string& input)
    {
        std::string result = input;

        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });

        return result;
    }

    static std::string trim(const std::string& s)
    {
        auto start = s.find_first_not_of(" \t\r\n");
        auto end = s.find_last_not_of(" \t\r\n");

        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static std::string stripQuotes(const std::string& s)
    {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            return s.substr(1, s.size() - 2);

        return s;
    }

    void MDFFile::assignTexturePath(int layer, const std::string& path)
    {
        std::string fixedPath = path;

        std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/');

        switch (layer)
        {
            case 0:
                texturePath[0] = fixedPath;
                break;
            case 1:
                texturePath[1] = fixedPath;
                break;
            case 2:
                texturePath[2] = fixedPath;
                break;
            case 3:
                texturePath[3] = fixedPath;
                break;
        }

        if (layer + 1 > textureCount)
        {
            textureCount = static_cast<uint8_t>(layer + 1);
        }
    }

    void MDFFile::checkAndSetRenderFlag(const std::string& keyword)
    {
        std::string lower = toLower(keyword);

        if (lower == "double")
            renderFlags |= RENDER_FLAG_DOUBLE;
        else if (lower == "linearfiltering")
            renderFlags |= RENDER_FLAG_LINEAR_FILTERING;
        else if (lower == "recalculatenormals")
            renderFlags |= RENDER_FLAG_RECALCULATE_NORMALS;
        else if (lower == "zfillonly")
            renderFlags |= RENDER_FLAG_Z_FILL_ONLY;
        else if (lower == "colorfillonly")
            renderFlags |= RENDER_FLAG_COLOR_FILL_ONLY;
        else if (lower == "notlit")
            renderFlags |= RENDER_FLAG_NOT_LIT;
        else if (lower == "disablez")
            renderFlags |= RENDER_FLAG_DISABLE_Z;
    }

    void MDFFile::setBlendType(int layer, const std::string& typeStr)
    {
        static const std::unordered_map<std::string, BlendType> map =
        {
            {"modulate", BLEND_TYPE_MODULATE},
            {"add", BLEND_TYPE_ADD},
            {"texturealpha", BLEND_TYPE_TEXTURE_ALPHA},
            {"currentalpha", BLEND_TYPE_CURRENT_ALPHA},
            {"currentalphaadd", BLEND_TYPE_CURRENT_ALPHA_ADD}
        };

        auto it = map.find(toLower(typeStr));

        if (it == map.end())
            return;

        switch (layer)
        {
            case 0:
                blendType[0] = it->second;
                break;
            case 1:
                blendType[1] = it->second;
                break;
            case 2:
                blendType[2] = it->second;
                break;
            case 3:
                blendType[3] = it->second;
                break;
        }
    }

    void MDFFile::setMaterialBlendType(const std::string& typeStr)
    {
        static const std::unordered_map<std::string, MaterialBlendType> map =
        {
            {"none", MATERIAL_BLEND_TYPE_NONE},
            {"alpha", MATERIAL_BLEND_TYPE_ALPHA},
            {"add", MATERIAL_BLEND_TYPE_ADD},
            {"alphaadd", MATERIAL_BLEND_TYPE_ALPHA_ADD}
        };

        auto it = map.find(toLower(typeStr));

        if (it != map.end())
        {
            materialBlendType = it->second;
        }
    }

    void MDFFile::setUVType(int layer, const std::string& typeStr)
    {
        static const std::unordered_map<std::string, UVType> map =
        {
            {"mesh", UV_TYPE_MESH},
            {"environment", UV_TYPE_ENVIRONMENT},
            {"drift", UV_TYPE_DRIFT},
            {"swirl", UV_TYPE_SWIRL},
            {"wavey", UV_TYPE_WAVEY}
        };

        auto it = map.find(toLower(typeStr));

        if (it == map.end())
            return;

        switch (layer)
        {
            case 0:
                uvType[0] = it->second;
                break;
            case 1:
                uvType[1] = it->second;
                break;
            case 2:
                uvType[2] = it->second;
                break;
            case 3:
                uvType[3] = it->second;
                break;
        }
    }

    void MDFFile::setSpeed(int layer, float u, float v)
    {
        float scaledU = u * 60.f;
        float scaledV = v * 60.f;

        switch (layer)
        {
            case 0:
                speedU[0] = scaledU;
                speedV[0] = scaledV;
                break;
            case 1:
                speedU[1] = scaledU;
                speedV[1] = scaledV;
                break;
            case 2:
                speedU[2] = scaledU;
                speedV[2] = scaledV;
                break;
            case 3:
                speedU[3] = scaledU;
                speedV[3] = scaledV;
                break;
        }
    }

    bool MDFFile::parseMDFFile(const std::string& rootPath, const std::string& materialPath)
    {
        std::ifstream file(rootPath + materialPath);
        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::istringstream stream(fileContent);
        std::string line;
        int currentLayer = 0;
        bool highQualitySection = false;

        if (!file)
            return false;

        while (std::getline(stream, line))
        {
            line = trim(line);

            if (line.empty())
                continue;

            std::istringstream linestream(line);
            std::string keyword;
            linestream >> keyword;

            std::string lowerKeyword = toLower(keyword);

            if (lowerKeyword == "textured")
            {
                materialType = MATERIAL_TYPE_TEXTURED;
                continue;
            }
            else if (lowerKeyword == "general")
            {
                materialType = MATERIAL_TYPE_GENERAL;
                continue;
            }
            else if (lowerKeyword == "clipper")
            {
                materialType = MATERIAL_TYPE_CLIPPER;
                continue;
            }
            else if (lowerKeyword == "highquality")
            {
                highQualitySection = true;
                continue;
            }

            if (lowerKeyword == "texture")
            {
                int layer = 0;
                std::string token;
                std::string restOfLine;
                
                linestream >> token;
                
                if (token.substr(0, 1) == "\"")
                {
                    std::getline(linestream, restOfLine);
                    restOfLine = token + restOfLine;
                }
                else
                {
                    layer = std::stoi(token);
                    std::getline(linestream, restOfLine);
                }

                std::string texPath = trim(stripQuotes(trim(restOfLine)));

                assignTexturePath(layer, rootPath + texPath);
            }
            else if (lowerKeyword == "uvtype")
            {
                int layer;
                std::string type;

                linestream >> layer >> type;

                setUVType(layer, type);
            }
            else if (lowerKeyword == "blendtype")
            {
                int layer;
                std::string blend;

                linestream >> layer >> blend;

                setBlendType(layer, blend);
            }
            else if (lowerKeyword == "speedu")
            {
                int layer;
                float val;

                linestream >> layer >> val;

                setSpeed(layer, val, 0.f);
            }
            else if (lowerKeyword == "speedv")
            {
                int layer;
                float val;

                linestream >> layer >> val;

                setSpeed(layer, 0.f, val);
            }
            else if (lowerKeyword == "speed")
            {
                float val;

                linestream >> val;

                for (int i = 0; i < 4; i++)
                {
                    setSpeed(i, val, val);
                }
            }
            else if (lowerKeyword == "glossmap")
            {
                std::string glossPath;
                linestream >> glossPath;
                
                glossMap = rootPath + stripQuotes(glossPath);
            }
            else if (lowerKeyword == "materialblendtype")
            {
                std::string type;
                linestream >> type;

                setMaterialBlendType(type);
            }
            else if (lowerKeyword == "specularpower")
            {
                linestream >> specularPower;
            }
            else if (lowerKeyword == "color")
            {
                linestream >> color.r >> color.g >> color.b >> color.a;
            }
            else if (lowerKeyword == "specular")
            {
                linestream >> specular.r >> specular.g >> specular.b >> specular.a;
            }
            else
            {
                checkAndSetRenderFlag(keyword);
            }
        }

        return true;
    }

    void MDFFile::debugPrint() const
    {
        LOG_DEBUG << "Material type: " << (int)materialType;

        for (int i = 0; i < 4; i++)
        {
            LOG_DEBUG << "Texture" << i << ": " << texturePath[i];
            LOG_DEBUG << "UVType" << i << ": " << (int)uvType[i];
            LOG_DEBUG << "BlendType" << i << ": " << (int)blendType[i];
            LOG_DEBUG << "SpeedU" << i << ": " << speedU[i];
            LOG_DEBUG << "SpeedV" << i << ": " << speedV[i];
        }

        LOG_DEBUG << "Gloss texture: " << glossMap;
        LOG_DEBUG << "Color: " << (unsigned int)color.r << " " << (unsigned int)color.g << " " << (unsigned int)color.b << " " << (unsigned int)color.a;
        LOG_DEBUG << "Specular: " << (unsigned int)specular.r << " " << (unsigned int)specular.g << " " << (unsigned int)specular.b << " " << (unsigned int)specular.a;
        LOG_DEBUG << "Specular power: " << specularPower;
        LOG_DEBUG << "Material blend type: " << (unsigned int)materialBlendType;
        LOG_DEBUG << "Render flags: " << (unsigned int)renderFlags;
        LOG_DEBUG << "Texture count: " << (unsigned int)textureCount;
    }
}
