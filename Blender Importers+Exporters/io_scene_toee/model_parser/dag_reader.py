import struct
import mathutils

class DAGModel:
    def __init__(self, file_path, apply_scale=False, scale_factor=0.0225):
        self.file_path = file_path
        self.apply_scale = apply_scale
        self.scale_factor = scale_factor
        self.vertices = []
        self.faces = []

    def read(self):
        with open(self.file_path, 'rb') as f:
            header_fmt = '<4f6I'
            header_size = struct.calcsize(header_fmt)
            header_data = struct.unpack(header_fmt, f.read(header_size))

            self.xOffset, self.yOffset, self.zOffset, self.boundingRadius, self.objectCount, dataStartOffset, vertexCount, faceCount, vertexDataOffset, faceDataOffset = header_data

            if (self.apply_scale):
                self.xOffset *= self.scale_factor
                self.yOffset *= self.scale_factor
                self.zOffset *= self.scale_factor

            # Read vertex positions
            f.seek(vertexDataOffset)
            for _ in range(vertexCount):
                x, y, z = struct.unpack('<3f', f.read(12))
                if (self.apply_scale):
                    x *= self.scale_factor
                    y *= self.scale_factor
                    z *= self.scale_factor
                self.vertices.append((x, y, z))

            # Read triangle indices
            f.seek(faceDataOffset)
            for _ in range(faceCount):
                indices = struct.unpack('<3H', f.read(6))
                self.faces.append(indices)

            self.vertex_count = vertexCount
            self.face_count = faceCount
