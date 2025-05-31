bl_info = {
    "name": "ToEE Model Import/Export",
    "author": "Aleist3r",
    "version": (0, 2),
    "blender": (3, 4, 1),
    "location": "File > Import/Export",
    "description": "Import/Export model and clipping files for Temple of Elemental Evil",
    "category": "Import-Export",
}

import bpy
from bpy.types import AddonPreferences
from bpy.props import StringProperty
from . import export_dag, import_skm, import_dag

class ToEEAddonPreferences(AddonPreferences):
    bl_idname = __name__

    data_path: StringProperty(
        name="Data Path",
        description="Base path for resolving relative texture/material references",
        subtype='DIR_PATH',
        default=""
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "data_path")

def menu_func_import(self, context):
    self.layout.operator(import_dag.ImportDAGModel.bl_idname, text="ToEE Clipping Model (.DAG)")
    self.layout.operator(import_skm.ImportToEEModel.bl_idname, text="ToEE Model (.SKM)")

def menu_func_export(self, context):
    self.layout.operator(export_dag.ExportDAGModel.bl_idname, text="ToEE Clipping Model (.DAG)")

def register():
    bpy.utils.register_class(ToEEAddonPreferences)
    export_dag.register()
    import_dag.register()
    import_skm.register()
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)

def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    import_skm.unregister()
    import_dag.unregister()
    export_dag.unregister()
    bpy.utils.unregister_class(ToEEAddonPreferences)

if __name__ == "__main__":
    register()
