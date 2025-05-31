import bpy
import struct
from mathutils import Vector

def write_dag(filepath, obj, apply_scale=False, scale_factor=0.0225):
    mesh = obj.data

    verts_loc = []
    verts_world = []

    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.quads_convert_to_tris()
    bpy.ops.object.mode_set(mode='OBJECT')

    center = obj.matrix_world.translation
    for v in mesh.vertices:
        v_loc = v.co
        v_world = v.co + center
        if apply_scale:
            v_loc /= scale_factor
            v_world /= scale_factor
        verts_loc.append(v_loc)
        verts_world.append(v_world)
    
    if apply_scale:
        center /= scale_factor

    xOffset, yOffset, zOffset = center.x, center.y, center.z

    center2d = Vector((xOffset, yOffset))
    bounding_radius = max((Vector((v.x, v.y)) - center2d).length for v in verts_world)

    vertex_count = len(mesh.vertices)
    face_count = len(mesh.polygons)

    header_size = 0x28  # 40 bytes
    vertex_data_offset = header_size
    face_data_offset = vertex_data_offset + vertex_count * 12

    with open(filepath, "wb") as f:
        f.write(struct.pack('<ffff', xOffset, yOffset, zOffset, bounding_radius))
        f.write(struct.pack('<IIIIII', 1, 0x18, vertex_count, face_count, vertex_data_offset, face_data_offset))  # objectCount

        for v in verts_loc:
            f.write(struct.pack('<fff', v.x, v.y, v.z))

        for poly in mesh.polygons:
            f.write(struct.pack('<3H', *poly.vertices))

