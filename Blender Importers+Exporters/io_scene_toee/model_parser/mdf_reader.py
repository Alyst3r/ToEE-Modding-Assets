import os

from ..utils import BlendType, ColorRGBA, MaterialBlendType, MaterialType, MDFMaterial, RenderFlags, UVType

def parse_mdf_file(root_path: str, material_path: str) -> MDFMaterial:
    material = MDFMaterial()
    full_path = os.path.join(root_path, material_path)

    if not os.path.exists(full_path):
        print(f"[ERROR] Material file not found: {full_path}")
        return material

    with open(full_path, 'r') as f:
        lines = f.readlines()

    for line in lines:
        line = line.strip()

        if not line or line.startswith("#"):
            continue

        tokens = line.split()
        keyword = tokens[0].lower()

        if keyword == "texture":
            if len(tokens) >= 3 and tokens[1].isdigit():
                layer = int(tokens[1])
                tex_path = " ".join(tokens[2:]).strip('"')
            else:
                layer = 0
                tex_path = " ".join(tokens[1:]).strip('"')
        
            if 0 <= layer < 4:
                full_tex_path = os.path.join(root_path, tex_path.replace("\\", "/"))
                material.texturePath[layer] = full_tex_path
                material.textureCount = max(material.textureCount, layer + 1)

        elif keyword == "uvtype":
            layer = int(tokens[1])
            type_str = tokens[2].lower()
            if 0 <= layer < 4:
                uv_map = {
                    "mesh": UVType.MESH,
                    "environment": UVType.ENVIRONMENT,
                    "drift": UVType.DRIFT,
                    "swirl": UVType.SWIRL,
                    "wavey": UVType.WAVEY
                }
                material.uvType[layer] = uv_map.get(type_str, UVType.MESH)

        elif keyword == "blendtype":
            layer = int(tokens[1])
            type_str = tokens[2].lower()
            if 0 <= layer < 4:
                blend_map = {
                    "modulate": BlendType.MODULATE,
                    "add": BlendType.ADD,
                    "texturealpha": BlendType.TEXTURE_ALPHA,
                    "currentalpha": BlendType.CURRENT_ALPHA,
                    "currentalphaadd": BlendType.CURRENT_ALPHA_ADD
                }
                material.blendType[layer] = blend_map.get(type_str, BlendType.MODULATE)

        elif keyword == "materialblendtype":
            type_str = tokens[1].lower()
            blend_map = {
                "none": MaterialBlendType.NONE,
                "alpha": MaterialBlendType.ALPHA,
                "add": MaterialBlendType.ADD,
                "alphaadd": MaterialBlendType.ALPHA_ADD
            }
            material.materialBlendType = blend_map.get(type_str, MaterialBlendType.NONE)

        elif keyword == "renderflags":
            for flag_token in tokens[1:]:
                flag = flag_token.lower()
                if flag == "double":
                    material.renderFlags |= RenderFlags.DOUBLE
                elif flag == "linearfiltering":
                    material.renderFlags |= RenderFlags.LINEAR_FILTERING
                elif flag == "recalculatenormals":
                    material.renderFlags |= RenderFlags.RECALCULATE_NORMALS
                elif flag == "zfillonly":
                    material.renderFlags |= RenderFlags.Z_FILL_ONLY
                elif flag == "colorfillonly":
                    material.renderFlags |= RenderFlags.COLOR_FILL_ONLY
                elif flag == "notlit":
                    material.renderFlags |= RenderFlags.NOT_LIT
                elif flag == "disablez":
                    material.renderFlags |= RenderFlags.DISABLE_Z

        elif keyword == "speedu":
            layer = int(tokens[1])
            val = float(tokens[2]) * 60.0
            if 0 <= layer < 4:
                material.speedU[layer] = val

        elif keyword == "speedv":
            layer = int(tokens[1])
            val = float(tokens[2]) * 60.0
            if 0 <= layer < 4:
                material.speedV[layer] = val

        elif keyword == "speed":  # Applies to all layers
            val = float(tokens[1]) * 60.0
            for i in range(4):
                material.speedU[i] = material.speedV[i] = val

        elif keyword == "color" and len(tokens) >= 5:
            material.color = ColorRGBA(*map(int, tokens[1:5]))

        elif keyword == "specular" and len(tokens) >= 5:
            material.specular = ColorRGBA(*map(int, tokens[1:5]))

        elif keyword == "specularpower" and len(tokens) >= 2:
            material.specularPower = float(tokens[1])

        elif keyword == "glossmap" and len(tokens) >= 2:
            material.glossMap = os.path.join(root_path, tokens[1].strip('"'))

        elif keyword in ["textured", "general", "clipper"]:
            type_map = {
                "textured": MaterialType.TEXTURED,
                "general": MaterialType.GENERAL,
                "clipper": MaterialType.CLIPPER
            }
            material.materialType = type_map[keyword]

    return material
