#pragma once

#include <string>

namespace MDF
{
	struct ColorRGBA
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 255;
	};

	struct MDFFile
	{
		std::string texturePath[4];
		std::string glossMap;
		uint8_t uvType[4] = { 0 };
		uint8_t blendType[4] = { 0 };
		float speedU[4] = { 0.f };
		float speedV[4] = { 0.f };
		ColorRGBA color;
		ColorRGBA specular; // doesn't seem to exist in any existing material file, glossMap texture is used instead
		float specularPower;
		uint8_t materialBlendType = 0;
		uint8_t renderFlags = 0;
		uint8_t textureCount = 0; // seems like valid count is 0-4
		uint8_t materialType = 0;

		enum UVType : uint8_t
		{
			UV_TYPE_MESH = 0,
			UV_TYPE_ENVIRONMENT = 1,
			UV_TYPE_DRIFT = 2,
			UV_TYPE_SWIRL = 3,
			UV_TYPE_WAVEY = 4
		};

		enum BlendType : uint8_t
		{
			BLEND_TYPE_MODULATE = 0,
			BLEND_TYPE_ADD = 1,
			BLEND_TYPE_TEXTURE_ALPHA = 2,
			BLEND_TYPE_CURRENT_ALPHA = 3,
			BLEND_TYPE_CURRENT_ALPHA_ADD = 4
		};

		enum MaterialBlendType : uint8_t
		{
			MATERIAL_BLEND_TYPE_NONE = 0,
			MATERIAL_BLEND_TYPE_ALPHA = 1,
			MATERIAL_BLEND_TYPE_ADD = 2,
			MATERIAL_BLEND_TYPE_ALPHA_ADD = 3
		};

		enum RenderFlags : uint8_t
		{
			RENDER_FLAG_DOUBLE = 0x01,
			//0x02 missing
			RENDER_FLAG_RECALCULATE_NORMALS = 0x04,
			RENDER_FLAG_Z_FILL_ONLY = 0x08,
			RENDER_FLAG_COLOR_FILL_ONLY = 0x10,
			RENDER_FLAG_NOT_LIT = 0x20,
			RENDER_FLAG_DISABLE_Z = 0x40,
			RENDER_FLAG_LINEAR_FILTERING = 0x80
		};

		enum MaterialType : uint8_t
		{
			MAERIAL_TYPE_TEXTURED = 0,
			MAERIAL_TYPE_GENERAL = 1,
			MAERIAL_TYPE_CLIPPER = 2
		};

		void assignTexturePath(int layer, const std::string& path);
		void checkAndSetRenderFlag(const std::string& keyword);
		void setBlendType(int layer, const std::string& typeStr);
		void setMaterialBlendType(const std::string& typeStr);
		void setUVType(int layer, const std::string& typeStr);
		void setSpeed(int layer, float u, float v);

		bool parseMDFFile(const std::string& rootPath, const std::string& materialPath);
		void debugPrint() const;
	};
}
