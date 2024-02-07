/*********************************************************************************************************************
 *
 * tools.h
 *
 * Minimal classes for Trackball and Camera
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef TOOLS_H
#define TOOLS_H

#include "volumeBase.h"

#define QT_NO_OPENGL_ES_2
#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#define NOMINMAX // avoid min*max macros to interfer with std::min/max from <windows.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>


        /*------------------------------------------------------------------------------------------------------------+
        |                                             MISC. CLASSES                                                   |
        +------------------------------------------------------------------------------------------------------------*/

struct Gbuffer
{
    GLuint colTex;
    GLuint normTex;
    GLuint posTex;
};

struct RayCasting
{
    GLuint frontPosTex;
    GLuint backPosTex;
    GLuint volTex;
};

struct MVPmatrices
{
    glm::mat4 modelMat;
    glm::mat4 viewMat;
    glm::mat4 projMat;
};


/*!
* \class Trackball
* \brief Handles trackball interaction
*/
class Trackball 
{
    private:


        double m_radius;         /*!< radius */
        bool m_tracking;         /*!< tracking activated/deactivated boolean state */
        glm::vec2 m_center;      /*!< 2D center's coords */
        glm::vec3 m_vStart;      /*!< 3D coords sarting position */
        glm::quat m_qStart;      /*!< quaternion starting position */
        glm::quat m_qCurrent;    /*!< quaternion current rotation */


    public:


        /*!
        * \fn Trackball
        * \brief Default constructor
        */
        Trackball() : m_radius(1.0),
                      m_center(glm::vec2(0.0f, 0.0f)),
                      m_tracking(false),
                      m_vStart(glm::vec3(0.0f, 0.0f, 1.0f)),
                      m_qStart(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
                      m_qCurrent(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
        {}

    
        /*!
        * \fn init
        * \brief Initialize trackball
        *
        * \param _width : viewport width
        * \param _height : viewport height
        */
        void init(int _width, int _height)
        {
            m_radius = double(std::min(_width, _height)) * 0.5f;
            m_center = glm::vec2(_width, _height) * 0.5f;
        }


        /*!
        * \fn reStart
        * \brief Set trackball in initial position
        */
        void reStart()
        {
            m_qCurrent = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }


        /*!
        * \fn mapMousePointToUnitSphere
        * \brief Maps 2D coords in screen space (i.e., mouse pointer) to 3D coords on unit sphere
        *
        * \param _point : 2D coords in screen space
        * \return 3D coords on unit sphere
        */
        glm::vec3 mapMousePointToUnitSphere(glm::vec2 _point)
        {
            // calculate the vector between center and point
            double x = _point[0] - m_center[0];
            double y = -_point[1] + m_center[1];
            double z = 0.0f;

            // the closer point is from center, the greater z is
            if (x * x + y * y < m_radius * m_radius / 2.0f) 
            {
                z = std::sqrt(m_radius * m_radius - (x * x + y * y));
            }
            else 
            {
                z = (m_radius * m_radius / 2.0f) / std::sqrt(x * x + y * y);
            }

            // normalize vector coords to get a point on unit sphere
            return glm::normalize(glm::vec3(x, y, z));
        }


        /*!
        * \fn startTracking
        * \brief Start trackball tracking from a given point
        */
        void startTracking(glm::vec2 _point)
        {
            m_center = _point; // !! @ 

            m_vStart = mapMousePointToUnitSphere(_point);
            m_qStart = glm::quat(m_qCurrent);
            m_tracking = true;
        }


        /*!
        * \fn stopTracking
        * \brief Stop trackball tracking
        */
        void stopTracking()
        {
            m_tracking = false;
        }


        /*!
        * \fn startTracking
        * \brief Rotate trackball to match a new given position (i.e., mouse movement)
        */
        void move(glm::vec2 _point)
        {
            // get new position
            glm::vec3 vCurrent = mapMousePointToUnitSphere(_point);
            // calculate rotation axis between init and new positions
            glm::vec3 rotationAxis = glm::cross(m_vStart, vCurrent);
            // calculate rotation angle between init and new positions
            float dotProduct = std::max(std::min(glm::dot(m_vStart, vCurrent), 1.0f), -1.0f);
            float rotationAngle = std::acos(dotProduct) * 2.0f;

            float eps = 0.01f;
            if (rotationAngle < eps) 
            {
                // no rotation is angle is small
                m_qCurrent = glm::quat(m_qStart);
            }
            else 
            {
                // Note: here we provide rotationAngle in radians. Older versions
                // of GLM (0.9.3 or earlier) require the angle in degrees.

                // build quaternion from rotation angle and axis
                glm::quat q = glm::angleAxis(rotationAngle, rotationAxis);
                q = glm::normalize(q);
                m_qCurrent = glm::normalize(glm::cross(q, m_qStart));
            }
        }


        /*!
        * \fn getRotationMatrix
        * \brief Get trackball orientation (quaternion) as a rotation matrix.
        */
        glm::mat4 getRotationMatrix()
        {
            return glm::mat4_cast(m_qCurrent);
        }


        /*!
        * \fn isTracking
        * \brief Tracking state getter
        */
        bool isTracking() { return m_tracking; }

}; // end class Trackball





/*!
* \class Camera
* \brief Handles camera matrices
*/
class Camera
{
    private:

        glm::mat4 m_projectionMatrix;     /*!< Perspective projection matrix */
        glm::mat4 m_viewMatrix;           /*!< View matrix */

        float m_nearPlane;                /*!< distance to near clip plane */
        float m_farPlane;                 /*!< distance to far clip plane */
        float m_fovy;                     /*!< field of view angle */
        float m_aspect;                   /*!< aspect ration */
        float m_zoomFactor;               /*!< factor applied to fov for zoom effect */
        float m_orthoOpening;             /*!< dimension of window to capture for orthognal projection */


    public:


        /*!
        * \fn Camera
        * \brief Default constructor
        */
        Camera() : m_projectionMatrix(glm::mat4(1.0f)),
                   m_viewMatrix(glm::mat4(1.0f)),
                   m_nearPlane(0.1f),
                   m_farPlane(50.0f),
                   m_fovy(45.0f),
                   m_aspect(3.0f/4.0f),
                   m_zoomFactor(1.0f)
        {}


        /*!
        * \fn init
        * \brief Initialize camera attributes  and matrices
        *
        * \param _near : distance to near clip plane
        * \param _far : distance to far clip plane
        * \param _fov : field of view angle
        * \param _zoomFactor : factor applied to fov for zoom effect
        * \param _width : viewport width
        * \param _height : viewport height
        * \param _camCoords : 3D coords of the camera position
        * \param _centerCoords : 3D coords of the scene's center (i.e., the position to look at)
        * \param _projType : projection type: perspective = 0, orthogonal = 1
        * \param _radScene : radius of the scene (for orthogonal projection only)
        */
        void init(float _near, float _far, float _fov, float _zoomFactor, int _width, int _height, glm::vec3 _camCoords, glm::vec3 _centerCoords, int _projType, float _radScene = 0.0f)
        {
            m_nearPlane = _near; 
            m_farPlane = _far;
            m_fovy = _fov;
            m_orthoOpening = _radScene*2.0f;

            initProjectionMatrix(_width, _height, _zoomFactor, _projType);
            initViewMatrix(_camCoords, _centerCoords);
        }


        /*!
        * \fn initProjectionMatrix
        * \brief Initialize the perspective projection matrix given the viewport dimensions and zoom factor 
        * \param _projType : 3projection type: perspective = 0, orthogonal = 1
        */
        void initProjectionMatrix(int _width, int _height, float _zoomFactor, int _projType)
        {
            m_aspect = (float)_width / (float)_height; 
            m_zoomFactor = _zoomFactor;

            if (_projType == 1 && m_orthoOpening == 0.0f)
                std::cerr << "[WARNING] Camera::initProjectionMatrix(): orthogonal projection matrix requires a non-null opening" << std::endl;

            if(_projType == 0)
                m_projectionMatrix = glm::perspective(glm::radians(m_fovy) * m_zoomFactor, m_aspect, m_nearPlane, m_farPlane);
            else if (_projType == 1)
            {
                // multiply width by aspect ratio to avoid stretching
                m_projectionMatrix = glm::ortho(-m_orthoOpening * m_aspect * m_zoomFactor, 
                                                 m_orthoOpening * m_aspect * m_zoomFactor, 
                                                -m_orthoOpening * m_zoomFactor, 
                                                 m_orthoOpening * m_zoomFactor, 
                                                 m_nearPlane, 
                                                 m_farPlane);
            }
            else
                std::cerr << "[WARNING] Camera::initProjectionMatrix(): projection type should be either 0 (perspective) or 1 (orthogonal):" << std::endl;
        }


        /*!
        * \fn initViewMatrix
        * \brief Initialize view matrix given the 3D coords of the camera position and the scene's center (i.e., the position to look at)
        */
        void initViewMatrix(glm::vec3 _camCoords, glm::vec3 _centerCoords)
        {
            // define up direction vector
            glm::vec3 upVec = glm::vec3(0, 1, 0);
            // avoid up vector and cam position to be aligned
            if (_camCoords.x == 0 && _camCoords.z == 0)
                upVec = glm::vec3(0, 0, 1);

            m_viewMatrix = glm::lookAt(_camCoords, _centerCoords, upVec);
        }


        /*!
        * \fn getProjectionMatrix
        * \brief ProjectionMatrix getter
        */
        glm::mat4 getProjectionMatrix() { return m_projectionMatrix; }


        /*!
        * \fn getViewMatrix
        * \brief ViewMatrix getter
        */
        glm::mat4 getViewMatrix() { return m_viewMatrix; }


}; // end class Camera




        /*------------------------------------------------------------------------------------------------------------+
        |                                            MISC. FUNCTIONS                                                  |
        +------------------------------------------------------------------------------------------------------------*/


namespace
{

    /*!
    * \fn sphericalToEuclidean
    * \brief Spherical coordinates to Euclidean coordinates
    * \param _spherical : spherical 3D coords
    * \return 3D Euclidean coords
    */
    glm::vec3 sphericalToEuclidean(glm::vec3 _spherical)
    {
        return glm::vec3(sin(_spherical.x) * cos(_spherical.y),
            sin(_spherical.y),
            cos(_spherical.x) * cos(_spherical.y)) * _spherical.z;
    }



    /*!
    * \fn readShaderSource
    * \brief read shader program and copy it in a string
    * \param _filename : shader file name
    * \return string containing shader program
    */
    std::string readShaderSource(const std::string& _filename)
    {
        std::ifstream file(_filename);
        std::stringstream stream;
        stream << file.rdbuf();

        return stream.str();
    }



    /*!
    * \fn showShaderInfoLog
    * \brief print out shader info log (i.e. compilation errors)
    * \param _shader : shader
    */
    void showShaderInfoLog(GLuint _shader)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(_shader, infoLogLength, &infoLogLength, &infoLog[0]);
        std::string infoLogStr(infoLog.begin(), infoLog.end());
        std::cerr << "[SHADER INFOLOG] " << infoLogStr << std::endl;
    }



    /*!
    * \fn showProgramInfoLog
    * \brief print out program info log (i.e. linking errors)
    * \param _program : program
    */
    void showProgramInfoLog(GLuint _program)
    {
        GLint infoLogLength = 0;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> infoLog(infoLogLength);
        glGetProgramInfoLog(_program, infoLogLength, &infoLogLength, &infoLog[0]);
        std::string infoLogStr(infoLog.begin(), infoLog.end());
        std::cerr << "[PROGRAM INFOLOG] " << infoLogStr << std::endl;
    }



    /*!
    * \fn loadShaderProgram
    * \brief load shader program from shader files
    * \param _vertShaderFilename : vertex shader filename
    * \param _fragShaderFilename : fragment shader filename
    */
    GLuint loadShaderProgram(const std::string& _vertShaderFilename, const std::string& _fragShaderFilename, const std::string& _vertHeader = "", const std::string& _fragHeader = "")
    {
        // read headers
        std::string vertHeaderSource, fragHeaderSource;
        vertHeaderSource = readShaderSource(_vertHeader);
        fragHeaderSource = readShaderSource(_fragHeader);


        // Load and compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        std::string vertexShaderSource = readShaderSource(_vertShaderFilename);
        if (!_vertHeader.empty())
        {
            // if headers are provided, add them to the shader
            const char* vertSources[2] = { vertHeaderSource.c_str(), vertexShaderSource.c_str() };
            glShaderSource(vertexShader, 2, vertSources, nullptr);
        }
        else
        {
            // if no header provided, the shader is contained in a single file
            const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
            glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
        }
        glCompileShader(vertexShader);
        GLint success = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cerr << "[ERROR] loadShaderProgram(): Vertex shader compilation failed:" << std::endl;
            showShaderInfoLog(vertexShader);
            glDeleteShader(vertexShader);
            return 0;
        }


        // Load and compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        std::string fragmentShaderSource = readShaderSource(_fragShaderFilename);
        if (!_fragHeader.empty())
        {
            // if headers are provided, add them to the shader
            const char* fragSources[2] = { fragHeaderSource.c_str(), fragmentShaderSource.c_str() };
            glShaderSource(fragmentShader, 2, fragSources, nullptr);
        }
        else
        {
            // if no header provided, the shader is contained in a single file
            const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
            glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
        }
        glCompileShader(fragmentShader);
        success = 0;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cerr << "[ERROR] loadShaderProgram(): Fragment shader compilation failed:" << std::endl;
            showShaderInfoLog(fragmentShader);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }


        // Create program object
        GLuint program = glCreateProgram();

        // Attach shaders to the program
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);


        // Link program
        glLinkProgram(program);

        // Check linking status
        success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            std::cerr << "[ERROR] loadShaderProgram(): Linking failed:" << std::endl;
            showProgramInfoLog(program);
            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        // Clean up
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        return program;
    }




    /*!
    * \fn buildScreenFBOandTex
    * \brief Generate a FBO and attach a texture to its color output (used for various screen texture generation)
    * \param _screenFBO : reference to id of FBO to generate
    * \param _screenTex : reference to id of texture to generate
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    */
    void buildScreenFBOandTex(GLuint& _screenFBO, GLuint& _screenTex, unsigned int _texWidth, unsigned int _texHeight)
    {

        // generate FBO 
        glGenFramebuffers(1, &_screenFBO);

        // generate texture
        glGenTextures(1, &_screenTex);
        glBindTexture(GL_TEXTURE_2D, _screenTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, _screenFBO);

        // attach textures to color output of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _screenTex, 0);


        // create and attach depth buffer (renderbuffer) to handle polygon occlusion properly (for 3D geometry rendering)
        unsigned int rboScreen;
        glGenRenderbuffers(1, &rboScreen);
        glBindRenderbuffer(GL_RENDERBUFFER, rboScreen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScreen);


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[ERROR] buildScreenFBOandTex(): screen FBO incomplete" << std::endl;
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    /*!
    * \fn build1DTex
    * \brief Create a 1D texture and writes data into it using a normal distribution / Gaussian ft (used to build transfer functions).
    * This texture is a lookup table / palette for 8b volume rendering, therefore the width is always 256
    * \param _1dTex : reference to id of texture to generate
    * \param _mean : mean value of the Gaussian
    * \param _radius : half-width of the Gaussian (= 2*std deviation, we consider the area that covers 95.45% of total integral)
    */
    void build1DTex(GLuint& _1dTex, unsigned int _mean, unsigned int _radius)
    {
        // create array of 256 values, sampled from a Gaussian curve
        float sigma = _radius * 0.5f; // std dev
        float factor = 1.0f / (float)(1.0f / (sigma * sqrt(2.0f * M_PI)) * exp(-1.0f * pow((float)_mean - (float)_mean, 2) / (2.0f * sigma * sigma)));

        std::vector<float> gaussValues;
        for (unsigned int i = 0; i < 256; i++)
        {
            float val = 1.0f / (float)(sigma * sqrt(2.0f * M_PI) * exp(-1.0f * pow((float)i - (float)_mean, 2) / (2.0f * sigma * sigma)));

            gaussValues.push_back(val * factor);
        }

        // generate 1D texture
        glGenTextures(1, &_1dTex);
        glBindTexture(GL_TEXTURE_1D, _1dTex);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R8, 256, 0, GL_RED, GL_FLOAT, &gaussValues[0]);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_1D, 0);
    }


    /*!
    * \fn build3DTex
    * \brief Create a 3D texture and copy volume data into it.
    * \param _volTex : reference to id of texture to generate
    * \param _vol : 3D image data (i.e., volume)
    * \param _useNearest : flag to indicate if texture uses GL_NEAREST param (if not, uses GL_LINEAR by default)
    */
    void build3DTex(GLuint& _volTex, VolumeBase<std::uint8_t>* _vol, bool _useNearest = false)
    {
        GLint param;
        _useNearest ? param = GL_NEAREST : param = GL_LINEAR;
        //_useNearest ? param = GL_NEAREST_MIPMAP_NEAREST : param = GL_LINEAR_MIPMAP_NEAREST;

        // generate 3D texture
        glGenTextures(1, &_volTex);
        glBindTexture(GL_TEXTURE_3D, _volTex);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, _vol->getDimensions().x, _vol->getDimensions().y, _vol->getDimensions().z, 0, GL_RED, GL_UNSIGNED_BYTE, _vol->getFront());
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, param);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, param);

        //glGenerateMipmap(GL_TEXTURE_3D);

        glBindTexture(GL_TEXTURE_3D, 0);
    }


    /*!
    * \fn update3DTex
    * \brief Update the content of  a 3D texture.
    * \param _volTex : pointer to id of texture
    * \param _vol : new 3D image data (i.e., volume)
    */
    void update3DTex(GLuint* _volTex, VolumeBase<std::uint8_t>* _vol)
    {

        glBindTexture(GL_TEXTURE_3D, *_volTex);

        glTexSubImage3D(GL_TEXTURE_3D, // target
            0, // level
            0, // x offset
            0, // y offset
            0, // z offset
            _vol->getDimensions().x,
            _vol->getDimensions().y,
            _vol->getDimensions().z,
            GL_RED, // format
            GL_UNSIGNED_BYTE, // type
            _vol->getFront()); // zeroed memory

        //glGenerateMipmap(GL_TEXTURE_3D);

        glBindTexture(GL_TEXTURE_3D, 0);
    }


    /*!
    * \fn buildScreenFBOandTex
    * \brief Generate a FBO and attach a texture to its color output (used for various screen texture generation)
    *        - can use a renderbuffer to handle depth buffering (for 3D geometry rendering)
             - can initialize texture with null alpha value (for TSD texture generation)
    * \param _screenFBO : pointer to id of FBO to generate
    * \param _screenTex : pointer to id of texture to generate
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    * \param _useRenderBuffer : use a renderbuffer or not
    * \param _nullAlpha : initialize texture with null alpha or not
    */
    void buildScreenFBOandTex(GLuint* _screenFBO, GLuint* _screenTex, unsigned int _texWidth, unsigned int _texHeight, bool _useRenderBuffer, bool _nullAlpha)
    {

        // generate FBO 
        glGenFramebuffers(1, _screenFBO);

        // generate texture
        glGenTextures(1, _screenTex);
        glBindTexture(GL_TEXTURE_2D, *_screenTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, *_screenFBO);

        // attach textures to color output of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_screenTex, 0);

        if (_nullAlpha)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0); //Only need to do this once.
            // make sure that alphae channel is set to zero everywhere (for gaussian blur mask in TSD)
            GLuint clearColor[4] = { 0, 0, 0, 0 };
            glClearBufferuiv(GL_COLOR, 0, clearColor);
        }

        if (_useRenderBuffer)
        {
            // create and attach depth buffer (renderbuffer) to handle polygon occlusion properly (for 3D geometry rendering)
            unsigned int rboScreen;
            glGenRenderbuffers(1, &rboScreen);
            glBindRenderbuffer(GL_RENDERBUFFER, rboScreen);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScreen);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[ERROR] buildScreenFBOandTex(): screen FBO incomplete" << std::endl;
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    /*!
    * \fn buildGbuffFBOandTex
    * \brief Generate a FBO and attach textures to 2 color outputs (used for G-buffer textures generation)
    *        Use a renderbuffer to handle depth buffering
    * \param _gFBO : reference to id of FBO to generate
    * \param _gBufferTex : reference to G-buffer (i.e., 3 color outputs for position, normal, and color)
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    */
    void buildGbuffFBOandTex(GLuint& _gFBO, Gbuffer& _gBufferTex, unsigned int _texWidth, unsigned int _texHeight)
    {
        // generate FBO 
        glGenFramebuffers(1, &_gFBO);

        // 1st texture (position buffer)
        glGenTextures(1, &_gBufferTex.posTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.posTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 2nd texture (normal buffer)
        glGenTextures(1, &_gBufferTex.normTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.normTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 3rd texture (color buffer)
        glGenTextures(1, &_gBufferTex.colTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.colTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, _gFBO);

        // attach textures to different color outputs of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _gBufferTex.posTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _gBufferTex.normTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, _gBufferTex.colTex, 0);

        // handle multiple color attachments
        GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);

        // create and attach depth buffer (renderbuffer)
        unsigned int rboGbuff;
        glGenRenderbuffers(1, &rboGbuff);
        glBindRenderbuffer(GL_RENDERBUFFER, rboGbuff);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboGbuff);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[ERROR] buildGbuffFBOandTex(): G-buffer FBO incomplete" << std::endl;
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }


    /*!
    * \fn buildRandKernel
    * \brief Generate a sample kernel in tangent-space hemisphere
    * \param _kernel : randomly sampled positions
    */
    void buildRandKernel(std::vector<glm::vec3>& _kernel)
    {
        // generate sample kernel 
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
        std::default_random_engine generator;
        _kernel.clear();

        int i = 0;
        while (i < 256)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator));
            if (glm::length(sample) <= 1.0)
            {
                sample = glm::normalize(sample);
                sample *= randomFloats(generator);
                float scale = (float)i / 64.0f;

                // scale samples s.t. they're more aligned to center of kernel
                scale = lerp(0.1f, 1.0f, scale * scale);
                _kernel.push_back(sample);
                i++;
            }
        }
    }


    /*!
    * \fn buildRandKernel
    * \brief Generate random rotation vectors, to be stored in a texture
    * \param _noiseTex : 2D texture to containing the output random vectors
    */
    void buildKernelRot(GLuint& _noiseTex)
    {
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
        std::default_random_engine generator;

        //create a 4x4 array of random rotation vectors oriented around the tangent-space surface normal
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 256; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);

            ssaoNoise.push_back(noise);
        }

        glGenTextures(1, &_noiseTex);
        glBindTexture(GL_TEXTURE_2D, _noiseTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 16, 16, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    /*!
    * \fn buildPerlinTex
    * \brief Generate pseudo random Perlin noise, to be stored in a texture
    * \param _noiseTex : 2D texture to containing the output random values
    */
    void buildPerlinTex(GLuint& _perlinTex)
    {
        std::vector<float> noise;
        for (unsigned int i = 0; i < 128; i++)
        {
            for (unsigned int j = 0; j < 128; j++)
            {
                float val = glm::perlin(glm::vec2((float)i / 128.0 * 30.0, (float)j / 128.0 * 30.0));
                // change range from [-1;1] to [0;1]
                val = (val + 1.0f) * 0.5f;

                noise.push_back( val );
            }
        }

        glGenTextures(1, &_perlinTex);
        glBindTexture(GL_TEXTURE_2D, _perlinTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 128, 128, 0, GL_RED, GL_FLOAT, &noise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

}

#endif // TOOLS_H