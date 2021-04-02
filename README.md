# Metal Gear Solid 4 Noesis Plugin


This is a plugin for [Noesis](https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91) which allows the user to view textured 3D Models and animations from the game Metal Gear Solid 4: Guns of the Patriots. As I do not know all of the material flags there textures may be assigned to the incorrect material slot.

I would also like to thank JinMar for his help on researching the shader flags.

![picture](https://github.com/Jayveer/MGS-MDN-Noesis/blob/master/model.png?raw=true)

### Latest Changes
 - Added different shader flags
 - Added Metal Gear Arcade support

### To Do
 - Add MGS3D support

##  Usage.

Drag the dll file into the plugins folder of your Noesis folder, run noesis and find and locate the MDN file you wish to view. Textures will be applied if the associated TXN and DLZ file can be found. It is best to use [Solideye](https://github.com/Jayveer/Solideye) to extract the files so they are in the correct folders and format. Slot files may require TXN and DLZ files from their Stage counterparts. 

There are a few menu options available, by default they are all unchecked. You will have to load a model again after changing these options for them to take effect.

##### Prompt for Motion Archive
This option will allow you to choose an Mtar file after the model has loaded. This allows you to view animations provided the bones match.

##### Highlight Missing Textures
This option will addd a fuschia colour to textures that are missing so that they are easier to identify.

##### Render Layer Map As Diffuse
The game blends certain diffuse maps together, as I cannot do this in Noesis I have given the option to render the layer map as the main diffuse for the times when the layer map is supposed to be the main diffuse.

##### Render Vertex Colour
Sometimes vertex colours cause some models to be transparent in places, because of this I decided to omit rendering them, however you can use this option to turn them on.