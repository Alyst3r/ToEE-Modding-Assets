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
