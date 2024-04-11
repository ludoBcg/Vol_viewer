/*********************************************************************************************************************
 *
 * main.cpp
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


// Standard includes 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define _USE_MATH_DEFINES
//#include <math.h>
#include <cstdlib>

#include "gui.h"

#include <tchar.h>
#include "aclapi.h"

// Window
GLFWwindow *m_window;           /*!<  GLFW window */
int m_winWidth = 800;           /*!<  window width */
int m_winHeight = 600;          /*!<  window height */
const unsigned int TEX_WIDTH = 2048, TEX_HEIGHT = 2048; /*!< textures dimensions  */

Trackball m_trackball;          /*!<  model trackball */
Trackball m_lightTrackball;     /*!<  light trackball */

// Scene
glm::vec3 m_centerCoords;       /*!<  coords of the center of the scene */
float m_radScene;               /*!< radius of the scene (i.e., diagonal of the BBox) */

// Light and cameras 
Camera m_camera3D;              /*!<  camera 3D view*/
Camera m_cameraA;               /*!<  camera Axial view (ortho) */
Camera m_cameraC;               /*!<  camera Coronal view (ortho) */
Camera m_cameraS;               /*!<  camera Sagittal view (ortho) */
float m_zoomFactor;             /*!<  zoom factor */
glm::vec3 m_camPos;             /*!<  camera position */
glm::vec3 m_lightDir;           /*!<  light direction (directional light source) */

// 3D objects
DrawableMesh* m_drawCube;       /*!<  drawable object: cube object */
DrawableMesh* m_drawScreenQuad; /*!<  drawable object: screen quad */
DrawableMesh* m_drawSliceA;     /*!<  drawable object: Axial slice */
DrawableMesh* m_drawSliceC;     /*!<  drawable object: Coronal slice */
DrawableMesh* m_drawSliceS;     /*!<  drawable object: Sagittal slice */

glm::mat4 m_modelMatrix;        /*!<  model matrix of the mesh */
    
GLuint m_defaultVAO;            /*!<  default VAO */

std::shared_ptr<VolumeImg> m_volume;

// FBOs
GLuint m_frontFaceFBO;          /*!< FBO for front face rendering of bounding geometry: renders fragment position coords as rgb colors m_frontPos */
GLuint m_backFaceFBO;           /*!< FBO for back face rendering of bounding geometry: renders fragment position coords as rgb colors m_frontPos */
GLuint m_gBufferFBO;            /*!< FBO for G-buffer: renders fragment position and normals coords as rgb colors into m_gPosition and m_gNormal */

// Textures
RayCasting m_rayCasting;        /*!< Textures for ray-casting  */
GLuint m_lookupTex;             /*!< TF 1D texture */
Gbuffer m_gBuf;                 /*!< screen-space textures for G-buffer  */

// shader programs
GLuint m_programBoundingGeom;   /*!< handle of the program object (i.e. shaders) for bounding geometry rendering */
GLuint m_programRayCast;        /*!< handle of the program object (i.e. shaders) for ray-casting rendering (MIP / alpha blending) */
GLuint m_programIsoSurf;        /*!< handle of the program object (i.e. shaders) for ray-casting rendering (isosurface) */
GLuint m_programHybrid;         /*!< handle of the program object (i.e. shaders) for ray-casting rendering (hybrid) */
GLuint m_programSlice;          /*!< handle of the program object (i.e. shaders) for slice rendering */
GLuint m_programQuad;           /*!< handle of the program object (i.e. shaders) for screen quad rendering */
GLuint m_programDeferred;       /*!< handle of the program object (i.e. shaders) for deferred screen space rendering of isosurface */


// Slice orientation
enum sliceOrient { AXIAL = 0, CORONAL = 1, SAGITTAL = 2 }; // axial = Z, coronal = Y, sagittal = X

// UI flags
UI m_ui;

float m_zoomFactA = 0.5f;               /*!<  scale factor to zoom of axial slice */
float m_zoomFactC = 0.5f;               /*!<  scale factor to zoom of coronal slice */
float m_zoomFactS = 0.5f;               /*!<  scale factor to zoom of sagittal slice */
glm::vec3 m_translatA(0.0f);            /*!<  translation for panning of axial slice */
glm::vec3 m_translatC(0.0f);            /*!<  translation for panning of coronal slice */
glm::vec3 m_translatS(0.0f);            /*!<  translation for panning of sagittal slice */
glm::vec3 m_translat3D(0.0f);           /*!<  translation for panning in 3D view */
bool m_startPanning3D = false;          /*! flag to indicate if panning is activated in 3D view */
bool m_startPanningA = false;           /*! flag to indicate if panning is activated in axial view */
bool m_startPanningC = false;           /*! flag to indicate if panning is activated in coronal view */
bool m_startPanningS = false;           /*! flag to indicate if panning is activated in sagittal view */
glm::vec2 m_prevMousePos(0.0f);
std::vector<glm::vec3> m_randKernel;    /*! random kernel for SSAO  */
GLuint m_noiseTex;                      /*! noise texture for SSAO  */
GLuint m_perlinTex;

std::vector<glm::ivec2> m_viewportPos;  /*! Store position (i.e., origin) of each viewport (use multiple viewport for split-screen) */
std::vector<glm::ivec2> m_viewportDim;  /*! Store dimension (i.e., resolution) of each viewport (use multiple viewport for split-screen) */


std::string shaderDir = "../../src/shaders/";   /*!< relative path to shaders folder  */


// Functions declaration

void initialize();
void calcViewportsCoords();
void initScene();
void setupImgui(GLFWwindow *window);
void update();
void renderBoundingGeom();
void renderRayCast();
void renderSlice();
void display();
void resizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow* window, unsigned int codepoint);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void runGUI();
int main(int argc, char** argv);





    /*------------------------------------------------------------------------------------------------------------+
    |                                                      INIT                                                   |
    +-------------------------------------------------------------------------------------------------------------*/


void initialize()
{   
    // init scene parameters
    m_zoomFactor = 1.0f;

    calcViewportsCoords();

    // Setup background color
    glClearColor(m_ui.backColor.r, m_ui.backColor.g, m_ui.backColor.b, 0.0f);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    
        
    // init model matrix
    m_modelMatrix = glm::mat4(1.0f);

    // new 3D image
    m_volume = std::make_shared<VolumeImg>();

    m_volume->volumeInit();
    //_tboig = new TBO(3 * _width * _height, GL_R8UI, "", nullptr);

    initScene();

    // setup mesh rendering
    m_drawCube = new DrawableMesh;
    m_drawCube->createUnitCubeVAO();

    m_drawSliceA = new DrawableMesh;
    m_drawSliceC = new DrawableMesh;
    m_drawSliceS = new DrawableMesh;
    m_drawSliceA->createSliceVAO(AXIAL);
    m_drawSliceC->createSliceVAO(CORONAL);
    m_drawSliceS->createSliceVAO(SAGITTAL);

    // setup screen quad rendering
    m_drawScreenQuad = new DrawableMesh;
    m_drawScreenQuad->createScreenQuadVAO();
    m_drawScreenQuad->setMaxSteps(glm::length(glm::vec3((float)m_volume->getDimensions().x,
                                                        (float)m_volume->getDimensions().y,
                                                        (float)m_volume->getDimensions().z)));


    // init shaders
    m_programBoundingGeom = loadShaderProgram(shaderDir + "boundingGeom.vert", shaderDir + "boundingGeom.frag");// renders 3D geometry with (XYZ) as colors, and writes results into positionTex
    m_programRayCast = loadShaderProgram(shaderDir + "rayCast.vert", shaderDir + "rayCast.frag");               // Performs ray-casting (MIP / alphabe blending)
    m_programIsoSurf = loadShaderProgram(shaderDir + "isoSurf.vert", shaderDir + "isoSurf.frag");               // Performs ray-casting (isosurface)
    m_programHybrid = loadShaderProgram(shaderDir + "hybrid.vert", shaderDir + "hybrid.frag");                  // Performs ray-casting (hybrid)
    m_programSlice = loadShaderProgram(shaderDir + "slice.vert", shaderDir + "slice.frag");                     // Render textured slices 
    m_programQuad = loadShaderProgram(shaderDir + "screenQuad.vert", shaderDir + "screenQuad.frag");            // Renders screenQuad with texture one
    m_programDeferred = loadShaderProgram(shaderDir + "deferred.vert", shaderDir + "deferred.frag");
    

    // build 3D texture from volume and FBO for raycasting
     build3DTex(m_rayCasting.volTex, m_volume.get());
    // build FBO and texture output for front and back face rendering of bounding geometry
    buildScreenFBOandTex(m_frontFaceFBO, m_rayCasting.frontPosTex, TEX_WIDTH, TEX_HEIGHT);
    buildScreenFBOandTex(m_backFaceFBO, m_rayCasting.backPosTex, TEX_WIDTH, TEX_HEIGHT);
    
    // build G-buffer FBO and textures
    buildGbuffFBOandTex(m_gBufferFBO, m_gBuf, TEX_WIDTH, TEX_HEIGHT);
    

    // build transfer function
    build1DTex(m_lookupTex);

    buildRandKernel(m_randKernel);
    buildKernelRot(m_noiseTex);
    buildPerlinTex(m_perlinTex);

    m_drawScreenQuad->setRandKernel(m_randKernel);
    m_drawScreenQuad->setNoiseTex(m_noiseTex);
    m_drawScreenQuad->setPerlinTex(m_perlinTex);

}

void calcViewportsCoords()
{
    // Viewport IDs
    // Full Screen:     Split screen:
    // +-------+        +---+---+
    // |       |        | 1 | 2 |
    // |   0   |        +---+---+
    // |       |        | 3 | 4 |
    // +-------+        +---+---+
    m_viewportPos = { glm::ivec2(0, 0),
                      glm::ivec2(0, m_winHeight / 2), glm::ivec2(m_winWidth / 2, m_winHeight / 2),
                      glm::ivec2(0, 0), glm::ivec2(m_winWidth / 2, 0) };
    m_viewportDim = { glm::ivec2(m_winWidth, m_winHeight),
                      glm::ivec2(m_winWidth / 2, m_winHeight / 2), glm::ivec2(m_winWidth / 2, m_winHeight / 2),
                      glm::ivec2(m_winWidth / 2, m_winHeight / 2), glm::ivec2(m_winWidth / 2, m_winHeight / 2) };
}


void initScene()
{
    glm::vec3 bBoxMin = m_volume->getOrigin();
    glm::vec3 bBoxMax = glm::vec3(m_volume->getDimensions()) * m_volume->getSpacing();

    if(bBoxMin != bBoxMax)
    {
        // set the center of the scene to the center of the bBox
        m_centerCoords = glm::vec3( (bBoxMin.x + bBoxMax.x) * 0.5f, (bBoxMin.y + bBoxMax.y) * 0.5f, (bBoxMin.z + bBoxMax.z) * 0.5f );
        m_centerCoords = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    m_radScene = glm::length(bBoxMax - bBoxMin) * 0.5f;
    m_radScene = 0.5f;

    // init camera position and light direction
    m_camPos = glm::vec3(m_radScene * 1.2f, m_radScene*0.6f, m_radScene*3.0f);
    m_lightDir = glm::vec3(0.0f, 0.0f, -1.0f);

    // init cameras
    m_camera3D.init(0.01f, m_radScene*8.0f, 45.0f, 1.0f, m_winWidth, m_winHeight, m_camPos, glm::vec3(0.0f, 0.0f, 0.0f), 0);
    m_cameraA.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactA, m_winWidth, m_winHeight, glm::vec3(0.0f, 0.0f, m_radScene * 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);
    m_cameraC.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactC, m_winWidth, m_winHeight, glm::vec3(0.0f, m_radScene * 6.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);
    m_cameraS.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactS, m_winWidth, m_winHeight, glm::vec3(m_radScene * 6.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);

    
    // init trackball
    m_trackball.init(m_winWidth, m_winHeight);
    m_lightTrackball.init(m_winWidth, m_winHeight);

    m_ui.sliceIdA = m_volume->getDimensions()[2] / 2;
    m_ui.sliceIdC = m_volume->getDimensions()[1] / 2;
    m_ui.sliceIdS = m_volume->getDimensions()[0] / 2;
}


void setupImgui(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Scale font for hdpi screen 
    ImFontConfig cfg;
    ImGui::GetIO().Fonts->AddFontDefault(&cfg)->Scale = 1.0f;
    
    ImGui::StyleColorsDark();

    // platform and renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}


    /*------------------------------------------------------------------------------------------------------------+
    |                                                     UPDATE                                                  |
    +-------------------------------------------------------------------------------------------------------------*/

void update()
{
    // update model matrix with trackball rotation
    m_modelMatrix = glm::translate( m_trackball.getRotationMatrix(), -m_centerCoords);
    m_lightDir = m_lightTrackball.getRotationMatrix() * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
}



    /*------------------------------------------------------------------------------------------------------------+
    |                                                     DISPLAY                                                 |
    +-------------------------------------------------------------------------------------------------------------*/


void renderBoundingGeom()
{
    // get matrices
    glm::mat4 modelMat = m_modelMatrix * m_volume->volumeComputeModelMatrix();
    glm::mat4 viewMat = m_camera3D.getViewMatrix();
    glm::mat4 projMat = m_camera3D.getProjectionMatrix();
    // apply translation after MVP for panning
    projMat = glm::translate(glm::mat4(1.0), m_translat3D) * projMat;
    MVPmatrices mvpMatrices = { modelMat, viewMat, projMat };

    // 1.
    // Render the front faces of the volume bounding box to a texture
    // via the frontFaceFBO
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // bind dedicated FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_frontFaceFBO);

    // resize viewport to output texture dimension
    glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

    // switch background to black to make sure empty fragments are not processed
    glClearColor(0.0f, 0.0f, 0.0f, 0.0);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    m_drawCube->drawBoundingGeom(m_programBoundingGeom, mvpMatrices);


    // 2.
    // Render the back faces of the volume bounding box to a texture
    // via the backFaceFBO
    glCullFace(GL_FRONT);

    // bind dedicated FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_backFaceFBO);

    // resize viewport to output texture dimension
    glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

    // switch background to black to make sure empty fragments are not processed
    glClearColor(0.0f, 0.0f, 0.0f, 0.0);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    m_drawCube->drawBoundingGeom(m_programBoundingGeom, mvpMatrices);


    // De-activate face culling
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);


    if (m_ui.isBackgroundWhite)
        glClearColor(1.0f, 1.0f, 1.0f, 0.0);
    else
        glClearColor(m_ui.backColor.r, m_ui.backColor.g, m_ui.backColor.b, 0.0f);
}


void renderRayCast()
{
    int viewID = 0;
    if (!m_ui.singleView)
    {
        viewID = 1;
    }

    if (m_ui.VRmode == 3 || m_ui.VRmode == 4)
    {
        // G-buffer for isosurface rendering

        // bind dedicated FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
        // resize viewport to output texture dimension
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
        glClearColor(m_ui.backColor.r, m_ui.backColor.g, m_ui.backColor.b, 0.0f);
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // get matrices
        glm::mat4 modelMat = m_modelMatrix * m_volume->volumeComputeModelMatrix();
        glm::mat4 viewMat = m_camera3D.getViewMatrix();
        glm::mat4 projMat = m_camera3D.getProjectionMatrix();
        // apply translation after MVP for panning
        projMat = glm::translate(glm::mat4(1.0), m_translat3D) * projMat;

        MVPmatrices mvpMatrices = { glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5)) * m_trackball.getRotationMatrix() * glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5)),
                                    viewMat, 
                                    projMat };
 
        GLuint program;
        m_ui.VRmode == 4 ? program = m_programHybrid : program = m_programIsoSurf;
        m_drawScreenQuad->drawIsoSurf(program, m_rayCasting, m_lookupTex, m_ui.isoValue, m_ui.isoValue2, mvpMatrices,
                                      m_lightDir, glm::vec2(m_viewportDim[viewID].x, m_viewportDim[viewID].y), m_ui.transparency);
    
        if (m_ui.isBackgroundWhite)
            glClearColor(1.0f, 1.0f, 1.0f, 0.0);
        else
            glClearColor(m_ui.backColor.r, m_ui.backColor.g, m_ui.backColor.b, 0.0f);
    }

    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // resize viewport to window dimensions
    glViewport(m_viewportPos[viewID].x, m_viewportPos[viewID].y, m_viewportDim[viewID].x, m_viewportDim[viewID].y);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // get matrices
    glm::mat4 modelMat = m_modelMatrix * m_volume->volumeComputeModelMatrix();
    glm::mat4 viewMat = m_camera3D.getViewMatrix();
    glm::mat4 projMat = m_camera3D.getProjectionMatrix();
    // apply translation after MVP for panning
    projMat = glm::translate(glm::mat4(1.0), m_translat3D) * projMat;

    // Perform ray-casting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_ui.showFrontTex)
        m_drawScreenQuad->drawScreenQuad(m_programQuad, m_rayCasting.frontPosTex);
    else if (m_ui.showBackTex)
        m_drawScreenQuad->drawScreenQuad(m_programQuad, m_rayCasting.backPosTex);
    else if (m_ui.VRmode == 3 || m_ui.VRmode == 4)
    {
        MVPmatrices mvpMatrices = { modelMat, viewMat, projMat };

        m_drawScreenQuad->drawDeferred(m_programDeferred, m_gBuf, mvpMatrices, glm::vec2(m_viewportDim[viewID].x, m_viewportDim[viewID].y));
    }
    else
        m_drawScreenQuad->drawRayCast(m_programRayCast, m_rayCasting, m_lookupTex, projMat * viewMat * modelMat, 
                                      glm::vec2(m_viewportDim[viewID].x, m_viewportDim[viewID].y), m_ui.transparency);

    glDisable(GL_BLEND);


}

void renderSlices3D()
{
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear window with background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int videwID = 0;
    if (!m_ui.singleView)
    {
        videwID = 1;
    }


    // resize viewport to window dimensions
    glViewport(m_viewportPos[videwID].x, m_viewportPos[videwID].y, m_viewportDim[videwID].x, m_viewportDim[videwID].y);


    // get matrices
    glm::mat4 modelMat = m_modelMatrix * m_volume->volumeComputeModelMatrixSlices();
    glm::mat4 viewMat = m_camera3D.getViewMatrix();
    glm::mat4 projMat = m_camera3D.getProjectionMatrix();
    // apply translation after MVP for panning
    projMat = glm::translate(glm::mat4(1.0), m_translat3D) * projMat;

    glEnable(GL_DEPTH_TEST);

    float translA = (float)m_ui.sliceIdA / (float)m_volume->getDimensions()[2];
    glm::mat4 translMatA = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 1.0f - translA));
    translA -= 0.5f;

    float translC = (float)m_ui.sliceIdC / (float)m_volume->getDimensions()[1];
    glm::mat4 translMatC = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 1.0f - translC, 0.0f));
    translC -= 0.5f;

    float translS = (float)m_ui.sliceIdS / (float)m_volume->getDimensions()[0];
    glm::mat4 translMatS = glm::translate(glm::mat4(1.0), glm::vec3(1.0f - translS, 0.0f, 0.0f));
    translS -= 0.5f;


    MVPmatrices mvpMatrices = { modelMat * glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, translA)), viewMat, projMat };
    m_drawSliceA->drawSlice(m_programSlice, mvpMatrices, translMatA, m_rayCasting.volTex, m_lookupTex);
    mvpMatrices.modelMat = modelMat * glm::translate(glm::mat4(1.0), glm::vec3(0.0f, translC, 0.0f));
    m_drawSliceC->drawSlice(m_programSlice, mvpMatrices, translMatC, m_rayCasting.volTex, m_lookupTex);
    mvpMatrices.modelMat = modelMat * glm::translate(glm::mat4(1.0), glm::vec3(translS, 0.0f, 0.0f));
    m_drawSliceS->drawSlice(m_programSlice, mvpMatrices, translMatS, m_rayCasting.volTex, m_lookupTex);

}


void renderSlice()
{
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int firstView = 0;
    int lastView = 0;
    if (!m_ui.singleView)
    {
        firstView = 2;
        lastView = 4;
    }
    else
    {
        // Clear window with background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // for each view
    for (int i = firstView; i <= lastView; i++)
    {
        // resize viewport to window dimensions
        glViewport(m_viewportPos[i].x, m_viewportPos[i].y, m_viewportDim[i].x, m_viewportDim[i].y);
        // Perform ray-casting
        glEnable(GL_DEPTH_TEST);

        // get matrices
        glm::mat4 modelMat = m_volume->volumeComputeModelMatrixSlices();

        if (i == 2 || (i == 0 && m_ui.mainViewOrient == 2))
        {
            glm::mat4 viewMat = m_cameraA.getViewMatrix();
            glm::mat4 projMat = m_cameraA.getProjectionMatrix();
            float translA = (float)m_ui.sliceIdA / (float)m_volume->getDimensions()[2];
            glm::mat4 texMat = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 1.0f - translA));
            glm::mat4 panMat = glm::translate(glm::mat4(1.0), m_translatA);
            MVPmatrices mvpMatrices = { panMat * modelMat, viewMat, projMat };
            m_drawSliceA->drawSlice(m_programSlice, mvpMatrices, texMat, m_rayCasting.volTex, m_lookupTex);
        }
        else if (i == 3 || (i == 0 && m_ui.mainViewOrient == 3))
        {
            glm::mat4 viewMat = m_cameraC.getViewMatrix();
            glm::mat4 projMat = m_cameraC.getProjectionMatrix();
            float translC = (float)m_ui.sliceIdC / (float)m_volume->getDimensions()[1];
            glm::mat4 texMat = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 1.0f - translC, 0.0f));
            glm::mat4 panMat = glm::translate(glm::mat4(1.0), m_translatC);
            MVPmatrices mvpMatrices = { panMat * modelMat, viewMat, projMat };
            m_drawSliceC->drawSlice(m_programSlice, mvpMatrices, texMat, m_rayCasting.volTex, m_lookupTex);
        }
        else if (i == 4 || (i == 0 && m_ui.mainViewOrient == 4))
        {
            glm::mat4 viewMat = m_cameraS.getViewMatrix();
            glm::mat4 projMat = m_cameraS.getProjectionMatrix();
            float translS = (float)m_ui.sliceIdS / (float)m_volume->getDimensions()[0];
            glm::mat4 texMat = glm::translate(glm::mat4(1.0), glm::vec3(1.0f - translS, 0.0f, 0.0f));
            glm::mat4 panMat = glm::translate(glm::mat4(1.0), m_translatS);
            MVPmatrices mvpMatrices = { panMat * modelMat, viewMat, projMat };
            m_drawSliceS->drawSlice(m_programSlice, mvpMatrices, texMat, m_rayCasting.volTex, m_lookupTex);
        }
        
    }
}


void display()
{
    if (!m_ui.singleView || (m_ui.singleView && m_ui.mainViewOrient == 1) )
    {
        if (m_ui.VR)
        {
            // 3D view
            renderBoundingGeom();
            renderRayCast();
        }
        else
        {
            renderSlices3D();
        }
    }
    
    if (!m_ui.singleView || (m_ui.singleView && (m_ui.mainViewOrient == 2 || m_ui.mainViewOrient == 3 || m_ui.mainViewOrient == 4)) )
    {
        renderSlice();
    }
}


    /*------------------------------------------------------------------------------------------------------------+
    |                                                CALLBACK METHODS                                             |
    +-------------------------------------------------------------------------------------------------------------*/


void resizeCallback(GLFWwindow* window, int width, int height)
{
    m_winWidth = width;
    m_winHeight = height;
    calcViewportsCoords();

    // re-init trackball and camera
    m_camera3D.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);
    m_cameraA.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactA, 1);
    m_cameraC.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactC, 1);
    m_cameraS.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactS, 1);
    m_trackball.init(m_winWidth, m_winHeight);
    m_lightTrackball.init(m_winWidth, m_winHeight);

    // keep drawing while resize
    update();

    display();

    // Swap between front and back buffer
    glfwSwapBuffers(m_window);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureKeyboard) { return; }  // Skip other handling

    // return to init positon when "R" pressed
    if (key == GLFW_KEY_R && action == GLFW_PRESS) 
    {
        // restart trackball
        m_trackball.reStart();
        m_lightTrackball.reStart();
        // re-init zoom
        m_zoomFactor = 1.0f;
        m_zoomFactA = m_zoomFactC = m_zoomFactS = 0.5f;
        // re-init panning
        m_translatA = m_translatC = m_translatS = m_translat3D = glm::vec3(0.0f);
        // update camera
        m_camera3D.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);
        m_cameraA.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactA, 1);
        m_cameraC.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactC, 1);
        m_cameraS.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactS, 1);
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        // reload shaders
        m_programBoundingGeom = loadShaderProgram(shaderDir + "boundingGeom.vert", shaderDir + "boundingGeom.frag");// renders 3D geometry with (XYZ) as colors, and writes results into positionTex
        m_programRayCast = loadShaderProgram(shaderDir + "rayCast.vert", shaderDir + "rayCast.frag");               // Performs ray-casting (MIP / alphabe blending)
        m_programIsoSurf = loadShaderProgram(shaderDir + "isoSurf.vert", shaderDir + "isoSurf.frag");               // Performs ray-casting (isosurface)
        m_programHybrid = loadShaderProgram(shaderDir + "hybrid.vert", shaderDir + "hybrid.frag");                  // Performs ray-casting (hybrid)
        m_programSlice = loadShaderProgram(shaderDir + "slice.vert", shaderDir + "slice.frag");                     // Render textured slices 
        m_programQuad = loadShaderProgram(shaderDir + "screenQuad.vert", shaderDir + "screenQuad.frag");
        m_programDeferred = loadShaderProgram(shaderDir + "deferred.vert", shaderDir + "deferred.frag");

    }
}


void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    if (ImGui::GetIO().WantTextInput) { return; }  // Skip other handling
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling   

    // get mouse cursor position
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if ((!m_ui.singleView && x < m_viewportDim[1].x && y < m_viewportDim[1].y)
        || (m_ui.singleView && m_ui.mainViewOrient == 1))
    {
        // activate/de-activate trackball with mouse button
        if (action == GLFW_PRESS)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
                m_trackball.startTracking(glm::vec2(x, y));
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
                m_lightTrackball.startTracking(glm::vec2(x, y));
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            {
                m_prevMousePos = glm::vec2(x, y);
                m_startPanning3D = true;
            }
        }
    }
    else
    {
        // activate 2D view panning with mouse button
        if (action == GLFW_PRESS)
        {
            if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            {

                m_prevMousePos = glm::vec2(x, y);
                if ((!m_ui.singleView && x > m_viewportDim[1].x && y < m_viewportDim[1].y)
                    || (m_ui.singleView && m_ui.mainViewOrient == 2))
                    m_startPanningA = true;
                else if ((!m_ui.singleView && x < m_viewportDim[1].x && y > m_viewportDim[1].y)
                    || (m_ui.singleView && m_ui.mainViewOrient == 3))
                    m_startPanningC = true;
                else if ((!m_ui.singleView && x > m_viewportDim[1].x && y > m_viewportDim[1].y)
                    || (m_ui.singleView && m_ui.mainViewOrient == 4))
                    m_startPanningS = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                std::cout << "pointer (X,Y) =  ( " << x << " , " << y << " ) --- " ;
                GLubyte *pixel = (GLubyte*)malloc(3);
                glReadPixels((int)x, m_winHeight - (int)y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
                std::cout << " pixel (RGB) = ( " << (int)pixel[0] << ", " << (int)pixel[1] << ", " << (int)pixel[2] << " )" << std::endl;
            }
        }
    }

    if (action == GLFW_RELEASE)
    {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            m_startPanningA = m_startPanningC = m_startPanningS = m_startPanning3D = false;
            m_prevMousePos = glm::vec2(0.0f, 0.0f);
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
            m_trackball.stopTracking();
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            m_lightTrackball.stopTracking();
    }
}


void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling   

    // get mouse cursor position
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    // update zoom in current viewport
    if ((!m_ui.singleView && x < m_viewportDim[1].x && y < m_viewportDim[1].y)
        || (m_ui.singleView && m_ui.mainViewOrient == 1))
    {
        // update zoom factor
        double newZoom = m_zoomFactor - yoffset / 10.0f;
        if (newZoom > 0.0f && newZoom < 2.0f)
        {
            m_zoomFactor -= (float)yoffset / 10.0f;
            // update camera
            m_camera3D.initProjectionMatrix(m_winWidth, m_winHeight, m_zoomFactor, 0);
        }

    }
    else if ((!m_ui.singleView && x > m_viewportDim[1].x && y < m_viewportDim[1].y)
        || (m_ui.singleView && m_ui.mainViewOrient == 2))
    {
        // update zoom factor
        double newZoom = m_zoomFactA - yoffset / 30.0f;
        if ((newZoom > 0.01f && yoffset > 0) || (newZoom < 1.5f && yoffset < 0))
        {
            m_zoomFactA -= (float)yoffset / 30.0f;
            m_cameraA.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactA, m_winWidth, m_winHeight, glm::vec3(0.0f, 0.0f, m_radScene * 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);
        }
    }
    else if ((!m_ui.singleView && x < m_viewportDim[1].x && y > m_viewportDim[1].y)
        || (m_ui.singleView && m_ui.mainViewOrient == 3))
    {
        // update zoom factor
        double newZoom = m_zoomFactC - yoffset / 30.0f;
        if ((newZoom > 0.01f && yoffset > 0) || (newZoom < 1.5f && yoffset < 0)) 
        {
            m_zoomFactC -= (float)yoffset / 30.0f;
            m_cameraC.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactC, m_winWidth, m_winHeight, glm::vec3(0.0f, m_radScene * 6.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);
        }
            
    }
    else if ((!m_ui.singleView && x > m_viewportDim[1].x && y > m_viewportDim[1].y)
        || (m_ui.singleView && m_ui.mainViewOrient == 4))
    {
        // update zoom factor
        double newZoom = m_zoomFactS - yoffset / 30.0f;
        if ((newZoom > 0.01f && yoffset > 0) || (newZoom < 1.5f && yoffset < 0))
        {
            m_zoomFactS -= (float)yoffset / 30.0f;
            m_cameraS.init(0.01f, m_radScene * 8.0f, 45.0f, m_zoomFactS, m_winWidth, m_winHeight, glm::vec3(m_radScene * 6.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, m_radScene);
        }
    }
}


void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling


    // rotate trackball according to mouse cursor movement
    if (m_trackball.isTracking())
    {
        m_trackball.move(glm::vec2(x, y));
    }
    else if (m_lightTrackball.isTracking())
    {
        m_lightTrackball.move(glm::vec2(x, y));
    }
    else if (m_startPanningA || m_startPanningC || m_startPanningS || m_startPanning3D)
    {
        float width = (float)m_winWidth;
        float height = (float)m_winHeight;
        if (!m_ui.singleView)
        {
            width *= 0.5f;
            height *= 0.5f;
        }


        if (((!m_ui.singleView && x > m_viewportDim[1].x && y < m_viewportDim[1].y)
            || (m_ui.singleView && m_ui.mainViewOrient == 2))
            && m_startPanningA)
        {
            // update axial translation vector
            m_translatA += glm::vec3(3.0f * (x - m_prevMousePos.x) / (float)width * m_zoomFactA, -2.0f * (y - m_prevMousePos.y) / (float)height * m_zoomFactA, 0.0f);
        }
        else if (((!m_ui.singleView && x < m_viewportDim[1].x && y > m_viewportDim[1].y)
            || (m_ui.singleView && m_ui.mainViewOrient == 3))
            && m_startPanningC)
        {
            // update coronal translation vector
            m_translatC += glm::vec3(-3.0f * (x - m_prevMousePos.x) / (float)width * m_zoomFactC, 0.0f, -2.0f * (y - m_prevMousePos.y) / (float)height * m_zoomFactC);
        }
        else if (((!m_ui.singleView && x > m_viewportDim[1].x && y > m_viewportDim[1].y)
            || (m_ui.singleView && m_ui.mainViewOrient == 4))
            && m_startPanningS)
        {
            // update coronal translation vector
            m_translatS += glm::vec3(0.0f, -2.0f * (y - m_prevMousePos.y) / (float)height * m_zoomFactS, -3.0f * (x - m_prevMousePos.x) / (float)width * m_zoomFactS);
        }
        else if (((!m_ui.singleView && x < m_viewportDim[1].x && y < m_viewportDim[1].y)
            || (m_ui.singleView && m_ui.mainViewOrient == 1))
            && m_startPanning3D)
        {
            // update 3D view translation vector
            m_translat3D += glm::vec3( 2.0f * (x - m_prevMousePos.x) / (float)width ,  -2.0f * (y - m_prevMousePos.y) / (float)height  , 0.0f);
        }
        m_prevMousePos = glm::vec2(x, y);
    }
}




    /*------------------------------------------------------------------------------------------------------------+
    |                                                      MAIN                                                   |
    +-------------------------------------------------------------------------------------------------------------*/

void runGUI()
{
    GUI(m_ui, *m_volume, m_rayCasting.volTex, *m_drawScreenQuad, *m_drawSliceA, *m_drawSliceC, *m_drawSliceS);
}

int main(int argc, char** argv)
{
    std::cout << std::endl
        << "Welcome to Vol_viewer" << std::endl << std::endl
        << "UI commands:" << std::endl
        << " - Mouse left button (3D view): trackball" << std::endl
        << " - Mouse right button (3D view): light trackball" << std::endl
        << " - Mouse middle button: panning" << std::endl
        << " - Mouse scroll: camera zoom" << std::endl
        << " - R: re-init trackball and cameras" << std::endl << std::endl
        << " - S: re-load shaders" << std::endl << std::endl
        << "Log:" << std::endl;

    /* Initialize GLFW and create a window */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // <-- activate this line on MacOS
    m_window = glfwCreateWindow(m_winWidth, m_winHeight, "Vol_viewer", nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, resizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    // init ImGUI
    setupImgui(m_window);


    // init GL extension wrangler
    glewExperimental = true;
    GLenum res = glewInit();
    if (res != GLEW_OK) 
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    
    std::cout << std::endl
              << "OpenGL version: " << glGetString(GL_VERSION) << std::endl
              << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
              << "Vendor: " << glGetString(GL_VENDOR) << std::endl;

    glGenVertexArrays(1, &m_defaultVAO);
    glBindVertexArray(m_defaultVAO);


    // call init function
    initialize();

    
    // main rendering loop
    while (!glfwWindowShouldClose(m_window)) 
    {
        // process events
        glfwPollEvents();
        // start frame for ImGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // build GUI
        runGUI();

        // idle updates
        update();
        // rendering
        display();
        
        // render GUI
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap between front and back buffer
        glfwSwapBuffers(m_window);
    }

    // Cleanup imGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Close window
    glfwDestroyWindow(m_window);
    glfwTerminate();

    std::cout << std::endl << "Bye!" << std::endl;
    
    return 0;
}
