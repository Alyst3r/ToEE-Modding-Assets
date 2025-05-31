import bpy
from bpy_extras.io_utils import ImportHelper
from bpy.types import Operator
from bpy.props import BoolProperty, FloatProperty
import os
from .model_parser import dag_reader
from .model_builder import clipper_builder
from . import utils

class ImportDAGModel(Operator, ImportHelper):
    bl_idname = "import_scene.toee_dag"
    bl_label = "Import ToEE Clipping Model"
    filename_ext = ".dag"
    filter_glob: bpy.props.StringProperty(default="*.dag", options={'HIDDEN'})

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
        self.filepath = utils.get_last_import_path()

        return ImportHelper.invoke(self, context, event)

    def execute(self, context):
        dag_path = self.filepath

        self.report({'INFO'}, f"Importing .DAG: {os.path.basename(dag_path)}")

        model = dag_reader.DAGModel(dag_path, apply_scale=self.apply_scale, scale_factor=self.scale_factor)
        model.read()

        mesh_obj = clipper_builder.build_dag_mesh(dag_path, model)
        utils.set_last_import_path(os.path.dirname(self.filepath))

        return {'FINISHED'}

def register():
    bpy.utils.register_class(ImportDAGModel)

def unregister():
    bpy.utils.unregister_class(ImportDAGModel)
