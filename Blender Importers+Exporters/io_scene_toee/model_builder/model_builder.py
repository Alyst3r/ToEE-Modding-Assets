import bpy
import os
import mathutils
from . import material_builder

def create_empty_and_armature(skm_path):
    model_name = os.path.splitext(os.path.basename(skm_path))[0]

    empty = bpy.data.objects.new(model_name, None)
    empty.empty_display_type = 'PLAIN_AXES'
    bpy.context.collection.objects.link(empty)

    arm_data = bpy.data.armatures.new(model_name + "_Armature")
    armature_obj = bpy.data.objects.new(model_name + "_Armature", arm_data)
    bpy.context.collection.objects.link(armature_obj)

    armature_obj.parent = empty
    return armature_obj, empty

def build_bones(armature_obj, bones_data):
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode='EDIT')
    edit_bones = armature_obj.data.edit_bones

    bone_lookup = {}
    for bone in bones_data:
        edit_bone = edit_bones.new(bone['name'])
        edit_bone.head = (0, 0, 0)
        edit_bone.tail = (0, 10, 0)
        bone_lookup[bone['name']] = edit_bone

    for i, bone in enumerate(bones_data):
        matrix = bone['matrix']
        edit_bone = bone_lookup[bone['name']]
        edit_bone.matrix = matrix

        if bone['parent'] >= 0:
            parent_name = bones_data[bone['parent']]['name']
            edit_bone.parent = bone_lookup[parent_name]
        
        edit_bone['bone_index'] = bone['index']

    bpy.ops.object.mode_set(mode='OBJECT')

def build_mesh(skm_path, model, armature_obj, empty, data_root):
    model_name = os.path.splitext(os.path.basename(skm_path))[0]
    
    mesh_data = bpy.data.meshes.new(model_name + "_Mesh")
    mesh_obj = bpy.data.objects.new(model_name + "_Mesh", mesh_data)
    bpy.context.collection.objects.link(mesh_obj)

    # Parent to empty
    mesh_obj.parent = empty

    # Create vertex positions and faces
    verts = [v['position'] for v in model.vertices]
    faces = [face[1] for face in model.faces]  # Only (v1, v2, v3)

    mesh_data.from_pydata(verts, [], faces)
    mesh_data.update()
    
    for poly in mesh_data.polygons:
        poly.use_smooth = True

    # UVs
    uv_layer = mesh_data.uv_layers.new(name="UVMap")
    for i, loop in enumerate(mesh_data.loops):
        uv = model.vertices[loop.vertex_index]['uv']
        uv_layer.data[i].uv = (uv[0], uv[1])

    # Vertex Groups
    for bone in model.bones:
        mesh_obj.vertex_groups.new(name=bone['name'])

    # Assign Weights
    for v_index, v in enumerate(model.vertices):
        for bone_id, weight in zip(v['bone_ids'], v['weights']):
            bone_name = model.bones[bone_id]['name']
            group = mesh_obj.vertex_groups.get(bone_name)
            if group:
                group.add([v_index], weight, 'REPLACE')

    # Add armature modifier
    modifier = mesh_obj.modifiers.new("Armature", 'ARMATURE')
    modifier.object = armature_obj

    # Create dummy materials (or based on actual model.materials later)
    for i in range(model.material_count):
        mat = bpy.data.materials.new(name=f"{model_name}_{i}")
        mesh_obj.data.materials.append(mat)

    # Assign materials to faces
    for poly, (mat_index, _) in zip(mesh_data.polygons, model.faces):
        poly.material_index = mat_index if mat_index < len(mesh_obj.data.materials) else 0

    if model.materials:
        material_builder.apply_materials(mesh_obj, model, skm_path, data_root)

    return mesh_obj
