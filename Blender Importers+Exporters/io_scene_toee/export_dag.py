import bpy
from bpy_extras.io_utils import ExportHelper
from bpy.types import Operator
from bpy.props import BoolProperty, FloatProperty
import os
from .model_writer import dag_writer
from . import utils

class ExportDAGModel(Operator, ExportHelper):
    bl_idname = "export_scene.toee_dag"
    bl_label = "Export ToEE Clipping Model"
    filename_ext = ".dag"
    filter_glob: bpy.props.StringProperty(default="*.dag", options={'HIDDEN'})

    selected_only: BoolProperty(
        name="Selected Only",
        description="Export only selected object",
        default=True,
    )

    apply_scale: BoolProperty(
        name="Apply Scale Factor",
        description="Scale model to match in-game scale",
        default=False,
    )

    scale_factor: FloatProperty(
        name="Scale Factor",
        description="Scale applied during export (0.01–2.0)",
        default=0.0225,
        min=0.01,
        max=2.0,
    )

    def invoke(self, context, event):
        self.filepath = utils.get_last_export_path()

        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        obj = None
        if self.selected_only:
            if not context.selected_objects:
                if not context.scene.objects:
                    self.report({'ERROR'}, "Scene is empty.")
                    return {'CANCELLED'}
                obj = context.scene.objects[0]
            else:
                obj = context.selected_objects[0]
        else:
            if not context.scene.objects:
                self.report({'ERROR'}, "Scene is empty.")
                return {'CANCELLED'}
            obj = context.scene.objects[0]

        if obj.type != 'MESH':
            self.report({'ERROR'}, "Selected object is not a mesh.")
            return {'CANCELLED'}

        dag_writer.write_dag(self.filepath, obj, apply_scale=self.apply_scale, scale_factor=self.scale_factor)
        utils.set_last_export_path(os.path.dirname(self.filepath))

        self.report({'INFO'}, f"Exported DAG: {self.filepath}")
        return {'FINISHED'}

def register():
    bpy.utils.register_class(ExportDAGModel)

def unregister():
    bpy.utils.unregister_class(ExportDAGModel)
