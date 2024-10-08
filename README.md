# Vol_viewer

A volume viewer for visualization of medical images

![img](img.png)

## 1. DATA

Vol_viewer can load 3D images using the .vtk file format. You can use [ITK-SNAP](http://www.itksnap.org/pmwiki/pmwiki.php) to export your data to the right format.

Images should be stored in the folder "Vol_viewer/data".


## 2. EXTERNAL DEPENDENCIES

All external dependencies are open-source libraries.

GLtools.h and other libraries are provided in the [libs](https://github.com/ludoBcg/libs) repository. 

External dependencies used for this project are:

* [GLEW (The OpenGL Extension Wrangler Library)](http://glew.sourceforge.net/)
  
* [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)

* [GLFW (Graphics Library Framework)](https://www.glfw.org/)

* [Dear ImGui (Immediate-mode Graphical User Interface)](https://github.com/ocornut/imgui)


## 3. COMPILATION


Vol_viewer is provided as a ready-to-build folder with a CMakeList. Make sure that it points to the correct *libs* directory, then use it to generate a project.
This project was developed with VisualStudio 2022.


## 4. SOURCES


* [1] J. Kruger and R. Westermann, “Acceleration Techniques for GPU-Based Volume Rendering,” Proceedings of the 14th IEEE Visualization, 2003, VIS '03, pp. 38-42.
* [2] https://learnopengl.com/PBR/Lighting
* [3] https://learnopengl.com/Advanced-Lighting/SSAO
* [4] M. Schnöller, "Real-Time Volume Rendering for Medical Images Visualization". Bsc. thesis, university of Innsbruck dpt. of computer science, 2021.
