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

#include "tools.h"


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
        
        /*! \fn setUseGammaCorrecFlag */
        inline void setUseGammaCorrecFlag(bool _useGammaCorrec) { m_useGammaCorrec = _useGammaCorrec; }
        /*! \fn setModeVR */
        inline void setModeVR(int _modeVR) { m_modeVR = _modeVR; }
        /*! \fn setMaxSteps */
        inline void setMaxSteps(int _maxSteps) { m_maxSteps = _maxSteps; }
        /*! \fn setUseAOFlag */
        inline void setUseAOFlag(bool _useAO) { m_useAO = _useAO; }
        /*! \fn setUseShadowFlag */
        inline void setUseShadowFlag(bool _useShadow) { m_useShadow = _useShadow; }
        /*! \fn setUseJitterFlag */
        inline void setUseJitterFlag(bool _useJitter) { m_useJitter = _useJitter; }
        /*! \fn setUseTF */
        inline void setUseTFFlag(bool _useTF) { m_useTF = _useTF; }
        /*! \fn setRandKernel */
        inline void setRandKernel(std::vector<glm::vec3> _randKernel) { m_randKernel = _randKernel; }
        /*! \fn setNoiseTex */
        inline void setNoiseTex(GLuint _noiseTex) { m_noiseTex = _noiseTex; }
        /*! \fn setPerlinTex */
        inline void setPerlinTex(GLuint _perlinTex) { m_perlinTex = _perlinTex; }
        /*! \fn setAmbientCol */
        inline void setAmbientCol(glm::vec3 _ambientCol) { m_ambientCol = _ambientCol; }

        /*! \fn getUseGammaCorrecFlag */
        inline bool getUseGammaCorrecFlag() { return m_useGammaCorrec; }
        /*! \fn getModeVR */
        inline bool getModeVR() { return m_modeVR; }
        /*! \fn getMaxSteps */
        inline bool getMaxSteps() { return m_maxSteps; }
        /*! \fn getUseAOFlag */
        inline bool getUseAOFlag() { return m_useAO; }
        /*! \fn getUseShadowFlag */
        inline bool getUseShadowFlag() { return m_useShadow; }
        /*! \fn getUseJitterFlag */
        inline bool getUseJitterFlag() { return m_useJitter; }
        /*! \fn getUseTFFlag */
        inline int getUseTFFlag() { return m_useTF; }


        /*------------------------------------------------------------------------------------------------------------+
        |                                                   MISC                                                      |
        +-------------------------------------------------------------------------------------------------------------*/

        /*!
        * \fn createScreenQuadVAO
        * \brief Create quad VAO and VBOs forscreen quad.
        */
        void createScreenQuadVAO();


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
        * \param _mvpMatrices : Model, View, and Projection matrices
        */
        void drawBoundingGeom(GLuint _program, MVPmatrices& _mvpMatrices);

        /*!
        * \fn drawScreenQuad
        * \brief Draw the screen quad, mapped with a given texture
        * \param _program : shader program
        * \param _tex : texture to map on the screen quad 
        */
        void drawScreenQuad(GLuint _program, GLuint _tex);

        /*!
        * \fn drawDeferred
        * \brief Draw the screen quad, assemble G-buffer textures
        * \param _program : shader program
        * \param _gBufferTex : G-buffer screen-space textures
        * \param _mvpMatrices : Model, View, and Projection matrices
        * \param _screenDims : current dimensions of screen 
        */
        void drawDeferred(GLuint _program, Gbuffer _gBufferTex, MVPmatrices& _mvpMatrices, glm::vec2 _screenDims);


        /*!
        * \fn drawRayCast
        * \brief Performs ray-casting
        * \param _program : shader program
        * \param _rayCastTex: reference to ray-casting set of textures (i.e., 3D texture with volume data + 2d textures for front and back face color rendering of bounding geometry)
        * \param _1dTex : 1D texture for transfer function (i.e., lookup table)
        * \param _mvpMat : MVP matrix
        * \param _transparency : transparency factor for alpha blending
        */
        void drawRayCast(GLuint _program, RayCasting& _rayCastTex, GLuint _1dTex,
                         glm::mat4 _mvpMat, float _transparency);

        /*!
        * \fn drawIsoSurf
        * \brief Performs ray-casting for iso-surface rendering
        * \param _program : shader program
        * \param _rayCastTex: reference to ray-casting set of textures (i.e., 3D texture with volume data + 2d textures for front and back face color rendering of bounding geometry)
        * \param _1dTex : 1D texture for transfer function (i.e., lookup table)
        * \param _isoValue: threshold value defining isosurface
        * \param _isoValue: threshold value defining second isosurface (for hybrid mode only)
        * \param _mvpMatrices : Model, View, and Projection matrices
        * \param _lightDir : light direction
        * \param _screenDims : current dimensions of screen 
        * \param _transparency : transparency factor for alpha blending (for hybrid mode only)
        */
        void drawIsoSurf(GLuint _program, RayCasting& _rayCastTex, GLuint _1dTex,
                         GLuint _isoValue, GLuint _isoValue2, MVPmatrices& _mvpMatrices, glm::vec3 _lightDir,
                         glm::vec2 _screenDims, float _transparency);

        /*!
        * \fn drawSlice
        * \brief Draw slice-quad with 3D texture mapping
        * \param _program : shader program
        * \param _mvpMatrices : Model, View, and Projection matrices
        * \param _tex3dMat :transformation to apply of tex coords (e.g., translation for slice scrolling)
        * \param _3dTex : 3D texture with volume data
        * \param _1dTex : 1D texture for transfer function (i.e., lookup table)
        */
        void drawSlice(GLuint _program, MVPmatrices& _mvpMatrices, glm::mat4 _tex3dMat, GLuint& _3dTex, GLuint _1dTex);


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

        bool m_useGammaCorrec;      /*!< flag to apply gamma correction or not */
        int m_modeVR;               /*!< VR mode (1 = MIP, 2 = alpha blending, 3 = isusurface, 4 = hybrid)*/
        int m_maxSteps;             /*!< max nb of steps for ray-casting (= diagonal length of volume box)*/

        bool m_vertexProvided;      /*!< flag to indicate if vertex coords are available or not */
        bool m_normalProvided;      /*!< flag to indicate if normals are available or not */
        bool m_colorProvided;       /*!< flag to indicate if colors are available or not */
        bool m_uvProvided;          /*!< flag to indicate if uv coords are available or not */
        bool m_tex3dProvided;       /*!< flag to indicate if 3D texture coords are available or not */
        bool m_indexProvided;       /*!< flag to indicate if indices are available or not */

        bool m_useAO;               /*!< flag to apply screen-space ambient occlusion or not */
        bool m_useShadow;           /*!< flag to apply shadows or not */
        bool m_useJitter;           /*!< flag to apply jittering or not */
        int m_useTF;                /*!< flag to apply Transfer Function or not */
        GLuint m_noiseTex;          /*!< index of noise texture */
        GLuint m_perlinTex;         /*!< index of perlin noise texture */
        std::vector<glm::vec3> m_randKernel;
        glm::vec3 m_ambientCol;     /*!< color used for ambient lighting and shadows */

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