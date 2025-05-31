import struct
import math
import mathutils

class SKMFile:
    def __init__(self, file_path):
        self.file_path = file_path
        self.bones = []
        self.materials = []
        self.vertices = []
        self.faces = []

    def read(self):
        with open(self.file_path, 'rb') as f:
            # Read header
            header_format = '<10I'
            header_size = struct.calcsize(header_format)
            header_data = struct.unpack(header_format, f.read(header_size))

            bone_count, bone_offset, mat_count, mat_offset, vert_count, vert_offset, face_count, face_offset, padding_0, padding_1 = header_data

            self.bone_count = bone_count
            self.material_count = mat_count
            self.vertex_count = vert_count
            self.face_count = face_count

            # Bones
            f.seek(bone_offset)
            for _ in range(bone_count):
                data = f.read(2 + 2 + 48 + 4 * 4 * 3)
                flags, parent = struct.unpack('<Hh', data[:4])
                name = data[4:52].split(b'\x00')[0].decode('utf-8')
                matrix_floats = struct.unpack('<12f', data[52:])
                inverse_world = mathutils.Matrix([
                    matrix_floats[0:4],
                    matrix_floats[4:8],
                    matrix_floats[8:12],
                    [0, 0, 0, 1]
                ])
                mat = inverse_world.inverted()
                self.bones.append({
                    'name': name,
                    'parent': parent,
                    'matrix': convert_to_blender_coords(mat),
                    'index': len(self.bones)
                })

            # Materials
            f.seek(mat_offset)
            for _ in range(mat_count):
                raw_path = f.read(128).split(b'\x00')[0].decode('utf-8')
                self.materials.append(raw_path)

            # Vertices
            f.seek(vert_offset)
            for _ in range(vert_count):
                data = f.read(4*4 + 4*4 + 2*4 + 2 + 2 + 2*6 + 4*6)
                pos = struct.unpack('<4f', data[0:16])
                normal = struct.unpack('<4f', data[16:32])
                uv = struct.unpack('<2f', data[32:40])
                weight_count = struct.unpack('<H', data[42:44])[0]
                bone_ids = struct.unpack('<6H', data[44:56])
                weights = struct.unpack('<6f', data[56:80])
                self.vertices.append({
                    'position': convert_vec3(pos),
                    'normal': convert_vec3(normal),
                    'uv': uv,
                    'bone_ids': bone_ids[:weight_count],
                    'weights': weights[:weight_count],
                })

            # Faces
            f.seek(face_offset)
            for _ in range(face_count):
                data = f.read(2 + 2*3)
                mat_index, v1, v2, v3 = struct.unpack('<4H', data)
                self.faces.append((mat_index, (v1, v2, v3)))

def convert_vec3(v):
    # Apply Y-up to Z-up conversion
    from mathutils import Vector
    return Vector((v[0], -v[2], v[1]))

def convert_to_blender_coords(matrix):
    # Apply transformation matrix rotation directly
    convert = mathutils.Matrix.Rotation(math.pi / 2, 4, 'X')
    return convert @ matrix
