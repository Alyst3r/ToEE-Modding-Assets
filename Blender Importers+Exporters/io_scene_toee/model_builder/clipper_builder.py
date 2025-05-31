import bpy
import os
import mathutils

def build_dag_mesh(dag_path, model):
    model_name = os.path.splitext(os.path.basename(dag_path))[0]

    mesh_data = bpy.data.meshes.new(model_name + "_Clipper")
    mesh_obj = bpy.data.objects.new(model_name + "_Clipper", mesh_data)
    bpy.context.collection.objects.link(mesh_obj)

    mesh_obj.location = mathutils.Vector((model.xOffset, model.yOffset, model.zOffset))

    verts = model.vertices
    faces = model.faces

    mesh_data.from_pydata(verts, [], faces)
    mesh_data.update()

    return mesh_obj
