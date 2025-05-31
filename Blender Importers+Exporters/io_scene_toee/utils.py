import os

from dataclasses import dataclass, field
from enum import Enum, IntFlag
from typing import List

class UVType(Enum):
    MESH = 0
    ENVIRONMENT = 1
    DRIFT = 2
    SWIRL = 3
    WAVEY = 4

class BlendType(Enum):
    MODULATE = 0
    ADD = 1
    TEXTURE_ALPHA = 2
    CURRENT_ALPHA = 3
    CURRENT_ALPHA_ADD = 4

class MaterialBlendType(Enum):
    NONE = 0
    ALPHA = 1
    ADD = 2
    ALPHA_ADD = 3


class RenderFlags(IntFlag):
    DOUBLE = 0x01
    RECALCULATE_NORMALS = 0x04
    Z_FILL_ONLY = 0x08
    COLOR_FILL_ONLY = 0x10
    NOT_LIT = 0x20
    DISABLE_Z = 0x40
    LINEAR_FILTERING = 0x80

class MaterialType(Enum):
    TEXTURED = 0
    GENERAL = 1
    CLIPPER = 2

@dataclass
class ColorRGBA:
    r: int = 0
    g: int = 0
    b: int = 0
    a: int = 255

@dataclass
class MDFMaterial:
    texturePath: List[str] = field(default_factory=lambda: [""] * 4)
    glossMap: str = ""
    uvType: List[int] = field(default_factory=lambda: [0] * 4)
    blendType: List[int] = field(default_factory=lambda: [0] * 4)
    speedU: List[float] = field(default_factory=lambda: [0.0] * 4)
    speedV: List[float] = field(default_factory=lambda: [0.0] * 4)
    color: ColorRGBA = ColorRGBA()
    specular: ColorRGBA = ColorRGBA()
    specularPower: float = 0.0
    materialBlendType: int = 0
    renderFlags: int = 0
    textureCount: int = 0
    materialType: int = 0

def get_last_import_path():
    try:
        addon_dir = os.path.dirname(os.path.realpath(__file__))
        path_file = os.path.join(addon_dir, "last_import_path.txt")
        with open(path_file, "r", encoding="utf-8") as f:
            path = f.read().strip()
            if os.path.isdir(path):
                return path
    except Exception:
        pass
    return ""

def set_last_import_path(path):
    try:
        clean_path = path.replace("\\", "/") + "/"
        addon_dir = os.path.dirname(os.path.realpath(__file__))
        path_file = os.path.join(addon_dir, "last_import_path.txt")
        with open(path_file, "w", encoding="utf-8") as f:
            f.write(clean_path)
    except Exception:
        pass

def get_last_export_path():
    try:
        addon_dir = os.path.dirname(os.path.realpath(__file__))
        path_file = os.path.join(addon_dir, "last_export_path.txt")
        with open(path_file, "r", encoding="utf-8") as f:
            path = f.read().strip()
            if os.path.isdir(path):
                return path
    except Exception:
        pass
    return ""

def set_last_export_path(path):
    try:
        clean_path = path.replace("\\", "/") + "/"
        addon_dir = os.path.dirname(os.path.realpath(__file__))
        path_file = os.path.join(addon_dir, "last_export_path.txt")
        with open(path_file, "w", encoding="utf-8") as f:
            f.write(clean_path)
    except Exception:
        pass
