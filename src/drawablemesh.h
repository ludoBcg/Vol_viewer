/*********************************************************************************************************************
 *
 * drawablemesh.h
 *
 * Buffer manager for mesh rendering
 * 
 * Vol_viewer
 * Ludovic Blache
 * 
 *********************************************************************************************************************/

#ifndef DRAWABLEMESH_H
#define DRAWABLEMESH_H

#define QT_NO_OPENGL_ES_2
#include <GL/glew.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

// Include GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>



// The attribute locations used in the vertex shader
enum AttributeLocation 
{
    POSITION = 0,
    NORMAL = 1,
    COLOR = 2,
    UV = 3,
    TEX3D = 4,
};



/*!
* \class DrawableMesh
* \brief Mesh datastructure with rendering functionalities
* Renders a TriMesh using a Blinn-Phong shading model and texture mapping
*/
class DrawableMesh
{
    public:

        /*------------------------------------------------------------------------------------------------------------+
        |                                        CONSTRUCTORS / DESTRUCTORS                                           |
        +------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn DrawableMesh
        * \brief Default constructor of DrawableMesh
        */
        DrawableMesh();


        /*!
        * \fn ~DrawableMesh
        * \brief Destructor of DrawableMesh
        */
        virtual ~DrawableMesh();


        /*------------------------------------------------------------------------------------------------------------+
        |                                              GETTERS/SETTERS                                                |
        +-------------------------------------------------------------------------------------------------------------*/
        
        /*! \fn setThreshMin */
        inline void setThreshMin(int _threshMin) { m_threshMin = _threshMin; }
        /*! \fn setThreshMax */
        inline void setThreshMax(int _threshMax) { m_threshMax = _threshMax; }
        /*! \fn setUseGammaCorrecFlag */
        inline void setUseGammaCorrecFlag(bool _useGammaCorrec) { m_useGammaCorrec = _useGammaCorrec; }
        /*! \fn setModeVR */
        inline void setModeVR(int _modeVR) { m_modeVR = _modeVR; }
        /*! \fn setMaxSteps */
        inline void setMaxSteps(int _maxSteps) { m_maxSteps = _maxSteps; }

        /*! \fn getThreshMin */
        inline int getThreshMin() { return m_threshMin; }
        /*! \fn getThreshMax */
        inline int getThreshMax() { return m_threshMax; }
        /*! \fn getUseGammaCorrecFlag */
        inline bool getUseGammaCorrecFlag() { return m_useGammaCorrec; }
        /*! \fn getModeVR */
        inline bool getModeVR() { return m_modeVR; }
        /*! \fn getMaxSteps */
        inline bool getMaxSteps() { return m_maxSteps; }


        /*------------------------------------------------------------------------------------------------------------+
        |                                                   MISC                                                      |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn createScreenQuadVAO
        * \brief Create quad VAO and VBOs forscreen quad.
        */
        void createScreenQuadVAO();

        void createCutPlaneVAO();

        /*!
        * \fn createUnitCubeVAO
        * \brief Create cube VAO and VBOs
        * minCorner = (0,0,0), maxCorner = (1,1,1), so that vertexcoords can be used as position color in G-buffer 
        */
        void createUnitCubeVAO();

        /*!
        * \fn createSliceVAO
        * \brief Create slices (i.e., 3 quads) VAO and VBOs
        * Slices are unit-quads (centered on origin) to be scaled to match Volume's dimension
        * \param _orientation : defines if the quad to build is an axial slice (=0), coronal (=1), or sagital (2)
        */
        void createSliceVAO(unsigned int _orientation);

        /*!
        * \fn drawBoundingGeom
        * \brief Draw the content of the mesh VAO
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        */
        void drawBoundingGeom(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat);

        /*!
        * \fn drawScreenQuad
        * \brief Draw the screen quad, mapped with a given texture
        * \param _program : shader program
        * \param _tex : texture to map on the screen quad 
        * \param _isBlurOn : true to activate Gaussian blur
        * \param _isGaussH : true for horizontal blur, false for vertical blur
        * \param _filterWidth : Guassian fildter width
        */
        void drawScreenQuad(GLuint _program, GLuint _tex, bool _isBlurOn, bool _isGaussH = true, int _filterWidth = 0 );

        /*!
        * \fn drawRayCast
        * \brief Performs ray-casting
        * \param _program : shader program
        * \param _3dTex : 3D texture with volume data
        * \param _frontTex : 2D texture with front face color rendering of bounding geometry
        * \param _backTex : 2D texture with back face color rendering of bounding geometry
        * \param _1dTex : 1D texture for transfer function (i.e., lookup table)
        */
        void drawRayCast(GLuint _program, GLuint _3dTex, GLuint _frontTex, GLuint _backTex, GLuint _1dTex);

        void drawRayCastReslice(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, glm::mat4 _rotMat, glm::vec3 _camPos, GLuint _3dTex, GLuint _frontTex, GLuint _backTex, float _d);


        void drawCutPlane(GLuint _program, glm::mat4 _modelMat, glm::mat4 _modelMat2, glm::mat4 _viewMat, glm::mat4 _projMat, GLuint _3dTex);
        void drawWF(GLuint _programW, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat);
        /*!
        * \fn drawSlice
        * \brief Draw slice-quad with 3D texture mapping
        * \param _program : shader program
        * \param _modelMat : model matrix
        * \param _viewMat :camera view matrix
        * \param _projMat :camera projection matrix
        * \param _tex3dMat :transformation to apply of tex coords (e.g., translation for slice scrolling)
        * \param _3dTex : 3D texture with volume data
        * \param _1dTex : 1D texture for transfer function (i.e., lookup table)
        */
        void drawSlice(GLuint _program, glm::mat4 _modelMat, glm::mat4 _viewMat, glm::mat4 _projMat, glm::mat4 _tex3dMat, GLuint _3dTex, GLuint _1dTex);

    protected:

        /*------------------------------------------------------------------------------------------------------------+
        |                                                ATTRIBUTES                                                   |
        +-------------------------------------------------------------------------------------------------------------*/

        GLuint m_meshVAO;           /*!< mesh VAO */
        GLuint m_defaultVAO;        /*!< default VAO */

        GLuint m_vertexVBO;         /*!< name of vertex 3D coords VBO */
        GLuint m_normalVBO;         /*!< name of normal vector VBO */
        GLuint m_colorVBO;          /*!< name of rgb color VBO */

        GLuint m_uvVBO;             /*!< name of UV coords VBO */
        GLuint m_tex3dVBO;          /*!< name of 3D TexCoords coords VBO */
        GLuint m_indexVBO;          /*!< name of index VBO */

        unsigned int m_numVertices; /*!< number of vertices in the VBOs */
        unsigned int m_numIndices;  /*!< number of indices in the index VBO */

        int m_threshMin;            /*!< lower threshold for rendering */
        int m_threshMax;            /*!< upper threshold for rendering */
        bool m_useGammaCorrec;      /*!< flag to apply gamma correction or not */
        int m_modeVR;               /*!< VR mode (1 = MIP, 2 = alpha blending, 3 = custom)*/
        int m_maxSteps;             /*!< max nb of steps for ray-casting (= diagonal length of volume box)*/

        bool m_vertexProvided;      /*!< flag to indicate if vertex coords are available or not */
        bool m_normalProvided;      /*!< flag to indicate if normals are available or not */
        bool m_colorProvided;       /*!< flag to indicate if colors are available or not */
        bool m_uvProvided;          /*!< flag to indicate if uv coords are available or not */
        bool m_tex3dProvided;       /*!< flag to indicate if 3D texture coords are available or not */
        bool m_indexProvided;       /*!< flag to indicate if indices are available or not */


        /*------------------------------------------------------------------------------------------------------------+
        |                                                   MISC                                                      |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn load2DTexture
        * \brief load a 2D image to be used as texture
        * \param _filename : name of texture image
        * \param _repeat : repeat (true) or clamptoedge (false)
        */
        //GLuint load2DTexture(const std::string& _filename, bool _repeat = false);

};
#endif // DRAWABLEMESH_H