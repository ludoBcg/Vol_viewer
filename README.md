# Vol_viewer

A Volume viewer for visualization of medical images

![img](https://github.com/ludoBcg/RT_lite/assets/84736834/608b0d13-e978-4f82-aa2f-6190478c4b01)


## 1. DATA

Vol_viewer can load 3D images using the .vtk file format.

Images should be stored in the folder "Vol_viewer/data".


## 2. EXTERNAL DEPENDENCIES

All external dependencies are open-source libraries.

For convenience, they should be copied in a "Vol_viewer/external" folder (see [RT_lite](https://github.com/ludoBcg/RT_lite) ).


* [GLEW (The OpenGL Extension Wrangler Library)](http://glew.sourceforge.net/)
  
* [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)

* [GLFW (Graphics Library Framework)](https://www.glfw.org/)

* [Dear ImGui (Immediate-mode Graphical User Interface)](https://github.com/ocornut/imgui)


## 3. COMPILATION


Vol_viewer is provided as a ready-to-build folder with a CMakeList. 
