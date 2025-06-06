# ToEE-Modding-Assets
Various utils that may be useful while modding Troika's Temple of Elemental Evil game.  

### 010 Editor Templates & Scripts
Self-explanatory, a bunch of 010 Editor utils allowing easier data manipulation.  

### toee_map_render_template_wip.blend
Made in Blender 3.4.1, not tested with newer versions. Work in progress, at some points in the future it will also contain various prefabs (rooms, furniture, etc.) based on original maps from the game. Also contains various notes related to map specifications. To render, insert keyframes (1-n) with proper positions on the grid then run Render->Render animation (or just use Ctrl+F12 shortcut). Each 256x256px frame will be saved in temporary folder (if I'm not mistaken it's `C:\tmp` by default). See example map render directory for preview.  
TODO: setup for z4xxxyyy and 8xxxyyy townmap files (maybe).  

### Utils src  
Source code for tools I'm making when I need to do specific tasks with certain file formats. Most are probably sloppily coded since I often reuse code snippets from tools I've written for other games years back (hey, if it works, it works, no need to reinvent the wheel).  
TODO: Wavefront obj to DAG (something I'll personally need for the project I'm working on, since it will most likely include maps where reusing existing clipping models wouldn't really be easy).  

### toee_icon.blend
Made in Blender 3.4.1 (again), it's basically recreation of original icon as ready to be rendered model. Various parameters of material could be adjusted to change such parameters like amount/shape of scratches, color, and so on. I've made it to render new icon for ToEE Model Viewer ;].  

### ToEE Model Viewer
Decided to mention it as its own header since it's kinda bigger util than others. In `Utils src` there is a new folder with source code of alpha version of model viewer I've started working on recently while on break from other stuff. For now it only displays raw geometry, without bones, materials or animations. I'm not providing compiled exe for now, if you really want to play with it, either compile yourself or grab pre-compiled x64 exe from Co8 forums.  
  
### Blender Importers+Exporters
For 3.4.1 (since it's what I'm using due to certain RW addons I use not having versions for 4.0+). I am not an addon dev (and not much of a Python coder in general) so, to be perfectly honest with you, I've fed ChatGPT with fine-tuned prompts and got working code for SKM import (correctly applies conversion from Y-up Z-forward coordinate system used by ToEE models to Z-up -Y-forward; for now materials are in most base form, just setting first texture without alpha data, only rest pose is imported). Also, DAG import and export should be fully functional. At some point I want to have full SKM+SKA import/export working (not going to be easy, thanks, Troika, some models have mismatching bone dependencies between SKM and SKA, sometimes SKA has more bones than SKM, and so on).  
  
### File format specifications
Various ToEE file format specifications written (or at least tried to be) in an accessible way.  