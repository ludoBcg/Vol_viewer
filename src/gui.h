/*********************************************************************************************************************
 *
 * gui.h
 * 
 * GUI widget
 *
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef GUI_H
#define GUI_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "volumeImg.h"
#include "drawablemesh.h"


std::string dataDir = "../../data/";         /*!< relative path to img files folder  */

struct UI {
    int mainViewOrient = 1;            /*! Defines which type of view is in main view (1=3D, 2=A, 3=C, 4=S) */
    glm::vec3 backColor = glm::vec3(0.5f, 0.5f, 0.5f); /*!< background color */
    bool isBackgroundWhite = false;   /*!< background color flag */
    bool isGammaCorrecOn = false;     /*!< Gamma correction flag */
    bool isAOOn = false;              /*!< Ambient Occlusion flag */
    bool isShadowOn = false;          /*!< Shadows flag */
    bool isJitterOn = false;          /*!< Jittering flag */
    bool useTF = false;          /*!< use Transfer Function flag */
    bool showFrontTex = false;        /*! Show front face texture of the bounding geometry*/
    bool showBackTex = false;         /*! Show back face texture of the bounding geometry*/
    bool singleView = true;           /*! Split screen or not*/
    bool VR = false;                  /*! Show VolumeRendering view or not (3D slices)*/
    int VRmode = 1;                   /*! Use MIP (1), alpha blending (2), isosurface (3), or hybrid (4) mode for VR*/
    bool useTexNearest = false;       /*! flag to indicate if texture uses GL_NEAREST param (if not, uses GL_LINEAR by default)*/
    int sliceIdA;                     /*! ID of the Axial slice to visualize*/
    int sliceIdC;                     /*! ID of the Coronal slice to visualize*/
    int sliceIdS;                     /*! ID of the Sagittal slice to visualize*/
    char fileName[256] = {};          /*! name of file to load */
    int isoValue = 38;                /*! threshold isosurface rendering */
    int isoValue2 = 255;              /*! threshold second isosurface rendering (hybrid mode only) */
    float transparency = 0.02f;       /*! opacity factor for alpha blending */
};

void loadFile(std::string _fileName, VolumeImg& _volume, GLuint& _volTex)
{
    _volume.volumeLoad(_fileName);
    //initScene();
    build3DTex(_volTex, &_volume);
}



void GUI( UI& _ui,
          VolumeImg& _volume,
          GLuint& _volTex,
          DrawableMesh& _drawScreenQuad,
          DrawableMesh& _drawSliceA,
          DrawableMesh& _drawSliceC,
          DrawableMesh& _drawSliceS )
{
    //bool test = true;
    //ImGui::ShowDemoWindow(&test);

    // Always show GUI at top-left corner when starting
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);


    if (ImGui::Begin("Settings"))
    {
        // ImGui frame rate measurement
        float frameRate = ImGui::GetIO().Framerate;
        ImGui::Text("FrameRate: %.3f ms/frame (%.1f FPS)", 1000.0f / frameRate, frameRate);

        ImGui::Separator();

        ImGui::Text("Format supported: .vtk, .raw");

        // filename
        ImGui::Text("File Name: ");
        ImGui::InputText(" ", _ui.fileName, sizeof(_ui.fileName));

        // import
        if (ImGui::Button("Load"))
        {
            loadFile(dataDir + std::string(_ui.fileName), _volume, _volTex);

            _drawScreenQuad.setMaxSteps(glm::length(glm::vec3((float)_volume.getDimensions().x,
                                                              (float)_volume.getDimensions().y,
                                                              (float)_volume.getDimensions().z)));

            _ui.sliceIdA = _volume.getDimensions()[2] / 2;
            _ui.sliceIdC = _volume.getDimensions()[1] / 2;
            _ui.sliceIdS = _volume.getDimensions()[0] / 2;
        }

        // Tab bar
        if (ImGui::BeginTabBar("tab bar"))
        {
            // -----------------------------------------------------------------------------------
            // Second tab: Visualization
            if (ImGui::BeginTabItem("Visu"))
            {
                if (ImGui::Checkbox("Show nearest voxel", &_ui.useTexNearest))
                {
                    // build 3D texture from volume and FBO for raycasting
                    build3DTex(_volTex, &_volume, _ui.useTexNearest);
                }

                if (ImGui::Checkbox("Gamma correction ", &_ui.isGammaCorrecOn))
                {
                    // Toggle gamma correction
                    _drawScreenQuad.setUseGammaCorrecFlag(_ui.isGammaCorrecOn);
                    _drawSliceA.setUseGammaCorrecFlag(_ui.isGammaCorrecOn);
                    _drawSliceC.setUseGammaCorrecFlag(_ui.isGammaCorrecOn);
                    _drawSliceS.setUseGammaCorrecFlag(_ui.isGammaCorrecOn);
                }

                if (!_ui.singleView || _ui.mainViewOrient == 1)
                {
                    ImGui::Checkbox("Volume rendering", &_ui.VR);
                    if (_ui.VR)
                    {
                        if (ImGui::RadioButton("MIP", &_ui.VRmode, 1))
                            _drawScreenQuad.setModeVR(1);
                        if (ImGui::RadioButton("alpha blending", &_ui.VRmode, 2))
                            _drawScreenQuad.setModeVR(2);
                        if (ImGui::RadioButton("isosurface", &_ui.VRmode, 3))
                            _drawScreenQuad.setModeVR(3);
                        if (ImGui::RadioButton("hybrid", &_ui.VRmode, 4))
                            _drawScreenQuad.setModeVR(4);

                        if (_ui.VRmode == 2 || _ui.VRmode == 4)
                        {
                            ImGui::SliderFloat("transparency", &_ui.transparency, 0.005f, 0.2f);
                        }

                        if (_ui.VRmode == 3 || _ui.VRmode == 4)
                        {
                            ImGui::SliderInt("Iso value", &_ui.isoValue, 0, 255);

                            if (_ui.VRmode == 4)
                                ImGui::SliderInt("Iso value 2", &_ui.isoValue2, 0, 255);

                            if (ImGui::Checkbox("Shadows", &_ui.isShadowOn))
                            {
                                _drawScreenQuad.setUseShadowFlag(_ui.isShadowOn);
                            }

                            if (ImGui::Checkbox("AO", &_ui.isAOOn))
                            {
                                _drawScreenQuad.setUseAOFlag(_ui.isAOOn);
                            }
                        }

                        if (_ui.VRmode == 3 || _ui.VRmode == 4)
                        {
                            if (ImGui::Checkbox("Jitter", &_ui.isJitterOn))
                            {
                                _drawScreenQuad.setUseJitterFlag(_ui.isJitterOn);
                            }
                        }

                        if (_ui.VRmode == 2 || _ui.VRmode == 4)
                        {
                            if (ImGui::Checkbox("TF", &_ui.useTF))
                            {
                                _drawScreenQuad.setUseTFFlag(_ui.useTF);
                            }
                        }
                    }
                }

                ImGui::EndTabItem();
            } // end tab Visualization

            // -----------------------------------------------------------------------------------
            // Third tab: Window views
            if (ImGui::BeginTabItem("View"))
            {
                //if (_ui.VR)
                //{
                //    ImGui::Checkbox("Show front face", &_ui.showFrontTex);
                //    ImGui::Checkbox("Show back face", &_ui.showBackTex);
                //}
                
                // change background color
                if (ImGui::Checkbox("White Background ", &_ui.isBackgroundWhite))
                {
                    glClear(GL_COLOR_BUFFER_BIT);
                    if (_ui.isBackgroundWhite)
                        glClearColor(1.0f, 1.0f, 1.0f, 0.0);
                    else
                        glClearColor(_ui.backColor.r, _ui.backColor.g, _ui.backColor.b, 0.0f);
                }

                ImGui::Checkbox("Single view", &_ui.singleView);
                if (_ui.singleView)
                {
                    ImGui::RadioButton("3D", &_ui.mainViewOrient, 1);
                    ImGui::RadioButton("Axial (Z-axis)", &_ui.mainViewOrient, 2);
                    ImGui::RadioButton("Axial (Y-axis)", &_ui.mainViewOrient, 3);
                    ImGui::RadioButton("Axial (X-axis)", &_ui.mainViewOrient, 4);
                }

                if (!_ui.singleView || _ui.mainViewOrient == 2 || (_ui.mainViewOrient == 1 && !_ui.VR))
                    ImGui::SliderInt("Axial (Z) slice", &_ui.sliceIdA, 1, _volume.getDimensions()[2]);
                if (!_ui.singleView || _ui.mainViewOrient == 3 || (_ui.mainViewOrient == 1 && !_ui.VR))
                    ImGui::SliderInt("Coronal (Y) slice", &_ui.sliceIdC, 1, _volume.getDimensions()[1]);
                if (!_ui.singleView || _ui.mainViewOrient == 4 || (_ui.mainViewOrient == 1 && !_ui.VR))
                    ImGui::SliderInt("Sagittal (X) slice", &_ui.sliceIdS, 1, _volume.getDimensions()[0]);

                ImGui::EndTabItem();
            } // end tab Window views
            ImGui::EndTabBar();
        } // end tab bar

    } // end "Settings"

    ImGui::End();

    // render
    ImGui::Render();
}

#endif // GUI_H