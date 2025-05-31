import bpy
import os

from ..model_parser import mdf_reader
from ..utils import MaterialType

def safe_load_image(path):
    if os.path.exists(path):
        try:
            return bpy.data.images.load(path, check_existing=True)
        except Exception as e:
            print(f"[ERROR] Could not load image: {path} -> {e}")
    return None

def apply_materials(mesh_obj, model, skm_path, data_root):
    model_name = os.path.splitext(os.path.basename(skm_path))[0]

    for i, mat_path in enumerate(model.materials):
        mdf = mdf_reader.parse_mdf_file(data_root, mat_path)
        tex_base = os.path.splitext(os.path.basename(mdf.texturePath[0]))[0] if mdf.texturePath[0] else f"{model_name}_{i}"

        if mdf.materialType == MaterialType.CLIPPER:
            print(f"[WARNING] Clipper material encountered ({mat_path}), using fallback dummy material.")
            continue

        elif mdf.materialType == MaterialType.TEXTURED:
            mat = bpy.data.materials.new(name=tex_base)
            mat.use_nodes = True
            mat.blend_method = 'OPAQUE'
            mat.shadow_method = 'OPAQUE'
            mat.use_backface_culling = False

            bsdf = mat.node_tree.nodes.get("Principled BSDF")
            tex_img_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
            tex_img_node.label = "Diffuse Texture"
            tex_img_node.location = (-300, 300)

            tex_path = mdf.texturePath[0]
            if os.path.exists(tex_path):
                try:
                    img = bpy.data.images.load(tex_path)
                    tex_img_node.image = img
                except Exception as e:
                    print(f"[ERROR] Failed to load texture: {tex_path} -> {e}")
            
            if tex_img_node.image:
                tex_img_node.image.alpha_mode = 'NONE'
                tex_img_node.image.colorspace_settings.name = 'sRGB'

            mat.node_tree.links.new(tex_img_node.outputs["Color"], bsdf.inputs["Base Color"])
            if img and img.channels == 4:
                mat.node_tree.links.new(tex_img_node.outputs["Alpha"], bsdf.inputs["Alpha"])

            bsdf.inputs["Specular"].default_value = 0.0

            if i < len(mesh_obj.data.materials):
                mesh_obj.data.materials[i] = mat
            else:
                mesh_obj.data.materials.append(mat)

        elif mdf.materialType == MaterialType.GENERAL:
            mat = bpy.data.materials.new(name=tex_base)
            mat.use_nodes = True
            mat.blend_method = 'OPAQUE'
            mat.shadow_method = 'OPAQUE'
            mat.use_backface_culling = False
        
            nodes = mat.node_tree.nodes
            links = mat.node_tree.links
            nodes.clear()
        
            output = nodes.new(type='ShaderNodeOutputMaterial')
            output.location = (400, 0)
        
            bsdf = nodes.new(type='ShaderNodeBsdfPrincipled')
            bsdf.location = (0, 0)
            bsdf.inputs["Specular"].default_value = 0.0
        
            if mdf.texturePath[0]:
                tex_node = nodes.new(type='ShaderNodeTexImage')
                tex_node.location = (-400, 0)
                tex_node.label = "Diffuse Texture"
                tex_node.image = safe_load_image(mdf.texturePath[0])

                if tex_node.image:
                    tex_node.image.alpha_mode = 'NONE'
                    tex_node.image.colorspace_settings.name = 'sRGB'

            links.new(tex_node.outputs["Color"], bsdf.inputs["Base Color"])

            links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])


            if i < len(mesh_obj.data.materials):
                mesh_obj.data.materials[i] = mat
            else:
                mesh_obj.data.materials.append(mat)
