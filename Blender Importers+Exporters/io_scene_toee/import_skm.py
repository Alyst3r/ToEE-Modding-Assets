import bpy
from bpy_extras.io_utils import ImportHelper
from bpy.types import Operator
import os
from .model_parser import skm_reader
from .model_builder import model_builder
from . import utils

class ImportToEEModel(Operator, ImportHelper):
    bl_idname = "import_scene.toee_model"
    bl_label = "Import ToEE Model"
    filename_ext = ".skm"
    filter_glob: bpy.props.StringProperty(default="*.skm", options={'HIDDEN'})

    include_animation: bpy.props.BoolProperty(
        name="Include Animation",
        description="Import animation from .SKA file",
        default=True
    )

    def invoke(self, context, event):
        self.filepath = utils.get_last_import_path()

        return ImportHelper.invoke(self, context, event)

    def draw(self, context):
        self.layout.prop(self, "include_animation")

    def execute(self, context):
        skm_path = self.filepath
        preferences = context.preferences.addons[__package__].preferences
        data_root = preferences.data_path
        ska_path = None

        if not data_root or not os.path.isdir(data_root):
            self.report({'WARNING'}, "Data Path is not set or invalid. Consider setting it in Preferences > Add-ons > ToEE Model Import/Export to correctly load materials and textures.")

        if self.include_animation:
            ska_path = os.path.splitext(skm_path)[0] + ".ska"
            if not os.path.exists(ska_path):
                self.report({'WARNING'}, f"Animation checked but .SKA file not found: {os.path.basename(ska_path)}")
                ska_path = None
            else:
                self.report({'INFO'}, f"With animation from .SKA: {os.path.basename(ska_path)}")

        self.report({'INFO'}, f"Importing .SKM: {os.path.basename(skm_path)}")
        if ska_path is None:
            self.report({'INFO'}, "No animation file used")

        self.report({'INFO'}, f"Loading SKM: {os.path.basename(skm_path)}")

        model = skm_reader.SKMFile(skm_path)
        model.read()

        self.report({'INFO'}, f"Vertices: {model.vertex_count}, Faces: {model.face_count}, Bones: {model.bone_count}")

        armature_obj, empty = model_builder.create_empty_and_armature(skm_path)
        model_builder.build_bones(armature_obj, model.bones)

        mesh_obj = model_builder.build_mesh(skm_path, model, armature_obj, empty, data_root)
        
        utils.set_last_import_path(os.path.dirname(self.filepath))
        
        return {'FINISHED'}

def register():
    bpy.utils.register_class(ImportToEEModel)

def unregister():
    bpy.utils.unregister_class(ImportToEEModel)
